#include "model.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <QDir>
#include <QFileInfo>

bool Model::load(const QString& resource)
{
    dir_ = QFileInfo{ resource }.dir().absolutePath().toStdString();

    Assimp::Importer importer{};
    const auto       scene =
        importer.ReadFile(resource.toStdString(), aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                                                      aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) return false;

    meshes_.clear();
    images_.clear();

    loadNode(scene, scene->mRootNode);

    return true;
}

bool Model::loadNode(const aiScene *scene, aiNode *node)
{
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
            materials.emplace_back(aiTextureType_DIFFUSE, dir_ + "/" + std::string(path.data, path.length));
        }

        for (unsigned di = 0; di < material->GetTextureCount(aiTextureType_SPECULAR); di++) {
            aiString path;
            material->GetTexture(aiTextureType_SPECULAR, di, &path);
            materials.emplace_back(aiTextureType_SPECULAR,
                                   dir_ + "/" + std::string(path.data, path.length));
        }

        for (unsigned di = 0; di < material->GetTextureCount(aiTextureType_HEIGHT); di++) {
            aiString path;
            material->GetTexture(aiTextureType_HEIGHT, di, &path);
            materials.emplace_back(aiTextureType_HEIGHT, dir_ + "/" + std::string(path.data, path.length));
        }

        for (unsigned di = 0; di < material->GetTextureCount(aiTextureType_AMBIENT); di++) {
            aiString path;
            material->GetTexture(aiTextureType_AMBIENT, di, &path);
            materials.emplace_back(aiTextureType_AMBIENT, dir_ + "/" + std::string(path.data, path.length));
        }

        for (const auto& mat : materials) {
            if (images_.contains(mat.path)) continue;

            images_[mat.path].load(QString::fromStdString(mat.path));
            images_[mat.path].flip();
        }

        meshes_.emplace_back(vertices, indices, materials);
    }

    for (uint32_t i = 0; i < node->mNumChildren; ++i) {
        loadNode(scene, node->mChildren[i]);
    }

    return true;
}

void Model::initialize(QRhi *rhi, QRhiRenderTarget *rt)
{
    for (auto& mesh : meshes_) {
        mesh.initialize(rhi, rt, images_);
    }
}

void Model::upload(QRhiResourceUpdateBatch *rub, const QMatrix4x4& mvp)
{
    for (auto& mesh : meshes_) {
        mesh.upload(rub, mvp, images_);
    }
}

void Model::draw(QRhiCommandBuffer *cb, const QRhiViewport& viewport)
{
    for (auto& mesh : meshes_) {
        mesh.draw(cb, viewport);
    }
}
