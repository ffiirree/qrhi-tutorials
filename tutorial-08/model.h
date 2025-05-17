#ifndef RHI_MODEL_H
#define RHI_MODEL_H

#include "mesh.h"

#include <assimp/scene.h>
#include <QString>

class Model
{
public:
    bool load(const QString& resource);

    bool loadNode(const aiScene *scene, aiNode *node);

    void initialize(QRhi *rhi, QRhiRenderTarget *rt);
    void upload(QRhiResourceUpdateBatch *rub, const QMatrix4x4& mvp);
    void draw(QRhiCommandBuffer *cb, const QRhiViewport& viewport);

private:
    std::string                   dir_{};
    std::vector<Mesh>             meshes_{};
    std::map<std::string, QImage> images_{};
};

#endif //! RHI_MODEL_H
