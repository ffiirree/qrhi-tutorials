#ifndef RHI_MODEL_H
#define RHI_MODEL_H

#include "mesh.h"
#include "render-item.h"

#include <assimp/scene.h>
#include <QString>

class Model : public RenderItem
{
public:
    explicit Model(const QString& path);

    bool load(const QString& resource);

    void load_node(const aiScene *scene, const aiNode *node, const QMatrix4x4& accumulated_transform);
    void load_mesh(const aiScene *scene, const aiMesh *mesh, QMatrix4x4 accumulated_transform);

    void create(QRhi *rhi, QRhiRenderTarget *rt) override;
    void upload(QRhiResourceUpdateBatch *rub, const QMatrix4x4& mvp) override;
    void draw(QRhiCommandBuffer *cb, const QRhiViewport& viewport) override;

private:
    std::string dir_{};
    bool        created_{};
    bool        uploaded_{};

    std::vector<std::unique_ptr<Mesh>>                  meshes_{};
    std::map<std::string, std::shared_ptr<QRhiTexture>> textures_{};
};

#endif //! RHI_MODEL_H
