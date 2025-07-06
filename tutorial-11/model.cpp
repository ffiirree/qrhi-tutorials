#include "model.h"

#include "utils.h"

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

    load_node(scene, scene->mRootNode, {});

    created_  = false;
    uploaded_ = false;

    return true;
}

void Model::load_node(const aiScene *scene, const aiNode *node, const QMatrix4x4& accumulated_transform)
{
    const auto transform = accumulated_transform * utils::to_qmatrix4x4(node->mTransformation);

    for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
        load_mesh(scene, scene->mMeshes[node->mMeshes[i]], transform);
    }

    for (uint32_t i = 0; i < node->mNumChildren; ++i) {
        load_node(scene, node->mChildren[i], transform);
    }
}

void Model::load_mesh(const aiScene *scene, const aiMesh *mesh, QMatrix4x4 accumulated_transform)
{
    std::vector<Vertex> vertices{};
    for (unsigned mi = 0; mi < mesh->mNumVertices; mi++) {
        Vertex vertex{
            .position = { mesh->mVertices[mi].x, mesh->mVertices[mi].y, mesh->mVertices[mi].z },
        };

        if (mesh->HasNormals()) {
            vertex.normal = { mesh->mNormals[mi].x, mesh->mNormals[mi].y, mesh->mNormals[mi].z };
        }

        if (mesh->mTextureCoords[0]) {
            vertex.coords = { mesh->mTextureCoords[0][mi].x, mesh->mTextureCoords[0][mi].y };
        }

        vertices.push_back(vertex);
    }

    std::vector<uint32_t> indices{};
    for (unsigned ii = 0; ii < mesh->mNumFaces; ii++) {
        const auto face = mesh->mFaces[ii];
        for (unsigned ij = 0; ij < face.mNumIndices; ij++) {
            indices.emplace_back(face.mIndices[ij]);
        }
    }

    std::vector<Material> materials{};
    const auto            material = scene->mMaterials[mesh->mMaterialIndex];

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

    meshes_.emplace_back(std::make_unique<Mesh>(vertices, indices, materials, accumulated_transform));
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

        for (const auto& mesh : meshes_) {
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

void Model::upload(QRhiResourceUpdateBatch *rub, const QMatrix4x4& mvp)
{
    if (!uploaded_) {
        for (auto& [path, tex] : textures_) {
            if (QImage image{}; image.load(QString::fromStdString(path))) {
                rub->uploadTexture(tex.get(), image);
            }
        }
        uploaded_ = true;
    }

    for (auto& mesh : meshes_) {
        mesh->upload(rub, mvp);
    }
}

void Model::draw(QRhiCommandBuffer *cb, const QRhiViewport& viewport)
{
    for (const auto& mesh : meshes_) {
        mesh->draw(cb, viewport);
    }
}
