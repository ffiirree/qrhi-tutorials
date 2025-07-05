#include "model.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <chrono>
#include <QDir>
#include <QFileInfo>

Model::Model(const QString& path) { load(path); }

bool Model::load(const QString& resource)
{
    dir_ = QFileInfo{ resource }.dir().absolutePath().toStdString();

    Assimp::Importer importer{};
    const auto       scene =
        importer.ReadFile(resource.toStdString(), aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                                                      aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        qDebug() << importer.GetErrorString();
        return false;
    }

    meshes_.clear();
    textures_.clear();

    loadNode(scene, scene->mRootNode, {});

    created_  = false;
    uploaded_ = false;

    return true;
}

bool Model::loadNode(const aiScene *scene, aiNode *node, aiMatrix4x4 transform)
{
    transform = transform * node->mTransformation;

    for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
        auto mesh = scene->mMeshes[node->mMeshes[i]];

        std::vector<Vertex> vertices{};
        for (unsigned mi = 0; mi < mesh->mNumVertices; mi++) {
            Vertex vertex{ mesh->mVertices[mi].x, mesh->mVertices[mi].y, mesh->mVertices[mi].z };

            if (mesh->HasNormals()) {
                vertex.normal[0] = mesh->mNormals[mi].x;
                vertex.normal[1] = mesh->mNormals[mi].y;
                vertex.normal[2] = mesh->mNormals[mi].z;
            }

            if (mesh->mTextureCoords[0]) {
                vertex.coord[0] = mesh->mTextureCoords[0][mi].x;
                vertex.coord[1] = mesh->mTextureCoords[0][mi].y;
            }

            vertices.push_back(vertex);
        }

        std::vector<uint32_t> indices{};
        for (unsigned ii = 0; ii < mesh->mNumFaces; ii++) {
            auto face = mesh->mFaces[ii];
            for (unsigned ij = 0; ij < face.mNumIndices; ij++) {
                indices.emplace_back(face.mIndices[ij]);
            }
        }

        std::vector<Material> materials{};
        auto                  material = scene->mMaterials[mesh->mMaterialIndex];

        for (unsigned di = 0; di < material->GetTextureCount(aiTextureType_DIFFUSE); di++) {
            aiString path;
            material->GetTexture(aiTextureType_DIFFUSE, di, &path);
            materials.emplace_back(aiTextureType_DIFFUSE, nullptr,
                                   dir_ + "/" + std::string(path.data, path.length));
            textures_[dir_ + "/" + std::string(path.data, path.length)].reset();
        }

        for (unsigned di = 0; di < material->GetTextureCount(aiTextureType_SPECULAR); di++) {
            aiString path;
            material->GetTexture(aiTextureType_SPECULAR, di, &path);
            materials.emplace_back(aiTextureType_SPECULAR, nullptr,
                                   dir_ + "/" + std::string(path.data, path.length));
            textures_[dir_ + "/" + std::string(path.data, path.length)].reset();
        }

        for (unsigned di = 0; di < material->GetTextureCount(aiTextureType_HEIGHT); di++) {
            aiString path;
            material->GetTexture(aiTextureType_HEIGHT, di, &path);
            materials.emplace_back(aiTextureType_HEIGHT, nullptr,
                                   dir_ + "/" + std::string(path.data, path.length));
            textures_[dir_ + "/" + std::string(path.data, path.length)].reset();
        }

        for (unsigned di = 0; di < material->GetTextureCount(aiTextureType_AMBIENT); di++) {
            aiString path;
            material->GetTexture(aiTextureType_AMBIENT, di, &path);
            materials.emplace_back(aiTextureType_AMBIENT, nullptr,
                                   dir_ + "/" + std::string(path.data, path.length));
            textures_[dir_ + "/" + std::string(path.data, path.length)].reset();
        }

        for (int bi = 0; bi < mesh->mNumBones; ++bi) {
            std::string name = mesh->mBones[bi]->mName.C_Str();
            qDebug() << name;

            int id = -1;
            if (bone_infos_.contains(name)) {
                id = bone_infos_[name].id;
            }
            else {
                BoneInfo bone{ name, static_cast<int>(bone_infos_.size()) };
                auto     m  = mesh->mBones[bi]->mOffsetMatrix;
                bone.offset = QMatrix4x4(m.a1, m.a2, m.a3, m.a4, m.b1, m.b2, m.b3, m.b4, m.c1, m.c2, m.c3,
                                         m.c4, m.d1, m.d2, m.d3, m.d4);
                bone_infos_[name] = bone;

                id = bone.id;
            }

            assert(id != -1);
            auto weights = mesh->mBones[bi]->mWeights;
            auto nw      = mesh->mBones[bi]->mNumWeights;

            for (int wi = 0; wi < nw; ++wi) {
                int   vertex_id = weights[wi].mVertexId;
                float weight    = weights[wi].mWeight;

                assert(vertex_id <= vertices.size());

                for (int z = 0; z < 4; ++z) {
                    if (vertices[vertex_id].bone_ids[z] < 0) {
                        vertices[vertex_id].weights[z]  = weight;
                        vertices[vertex_id].bone_ids[z] = id;
                        break;
                    }
                }
            }
        }

        meshes_.emplace_back(std::make_unique<Mesh>(vertices, indices, materials, transform));
    }

    for (uint32_t i = 0; i < node->mNumChildren; ++i) {
        loadNode(scene, node->mChildren[i], transform);
    }

    return true;
}

void Model::create(QRhi *rhi, QRhiRenderTarget *rt)
{
    if (!created_) {
        for (auto& [key, value] : textures_) {
            QImage image{};
            if (!image.load(QString::fromStdString(key))) continue;

            value.reset(rhi->newTexture(QRhiTexture::RGBA8, image.size()));
            value->create();
        }

        for (auto& mesh : meshes_) {
            for (auto& mat : mesh->materials) {
                mat.texture = textures_[mat.path];
            }
        }
        created_ = true;
    }

    for (auto& mesh : meshes_) {
        mesh->create(rhi, rt);
    }
}

void Model::upload(QRhiResourceUpdateBatch *rub, const QMatrix4x4& mvp, const std::vector<QMatrix4x4>& bm)
{
    if (!uploaded_) {
        for (auto& [path, tex] : textures_) {
            QImage image{};
            if (image.load(QString::fromStdString(path))) {
                rub->uploadTexture(tex.get(), image);
            }
        }
        uploaded_ = true;
    }

    for (auto& mesh : meshes_) {
        mesh->upload(rub, mvp, bm);
    }
}

void Model::draw(QRhiCommandBuffer *cb, const QRhiViewport& viewport)
{
    for (auto& mesh : meshes_) {
        mesh->draw(cb, viewport);
    }
}
