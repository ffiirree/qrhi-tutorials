#ifndef RHI_MESH_H
#define RHI_MESH_H

#include "render-item.h"

#include <assimp/material.h>

struct Vertex
{
    float position[3]{};
    float normal[3]{};
    float coord[2]{};
};

struct Material
{
    aiTextureType                type{};
    std::shared_ptr<QRhiTexture> texture{};
    std::string                  path{};
};

class Mesh : public RenderItem
{
public:
    Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::vector<Material> materials);

    Mesh(Mesh&&) = default;

    void create(QRhi *rhi, QRhiRenderTarget *rt) override;
    void upload(QRhiResourceUpdateBatch *rub, const QMatrix4x4& mvp) override;
    void draw(QRhiCommandBuffer *cb, const QRhiViewport& viewport) override;

public:
    std::vector<Vertex>   vertices{};
    std::vector<uint32_t> indices{};
    std::vector<Material> materials{};

private:
    QRhi *rhi_{};

    std::unique_ptr<QRhiGraphicsPipeline>       pipeline_{};
    std::unique_ptr<QRhiBuffer>                 vbuf_{};
    std::unique_ptr<QRhiBuffer>                 ibuf_{};
    std::unique_ptr<QRhiBuffer>                 ubuf_{};
    std::unique_ptr<QRhiSampler>                sampler_{};
    std::unique_ptr<QRhiShaderResourceBindings> srb_{};

    bool uploaded_{};
};

#endif //! RHI_MESH_H
