#ifndef RHI_MESH_H
#define RHI_MESH_H

#include <assimp/material.h>
#include <rhi/qrhi.h>

struct Vertex
{
    float position[3]{};
    float normal[3]{};
    float coord[2]{};
};

struct Material
{
    aiTextureType type{};
    std::string   path{};
};

class Mesh
{
public:
    Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::vector<Material> materials);

    Mesh(Mesh&&) = default;

    void initialize(QRhi *rhi, QRhiRenderTarget *rt, const std::map<std::string, QImage>& images);
    void upload(QRhiResourceUpdateBatch *rub, const QMatrix4x4& mvp, const std::map<std::string, QImage>& images);
    void draw(QRhiCommandBuffer *cb, const QRhiViewport& viewport);

public:
    std::vector<Vertex>   vertices{};
    std::vector<uint32_t> indices{};
    std::vector<Material> materials{};

private:
    QRhi *rhi_{};

    std::unique_ptr<QRhiGraphicsPipeline>       pipeline_{};
    std::unique_ptr<QRhiBuffer>                 vbuf_{};
    std::unique_ptr<QRhiBuffer>                 ubuf_{};
    std::unique_ptr<QRhiSampler>                sampler_{};
    std::unique_ptr<QRhiShaderResourceBindings> srb_{};
    std::vector<std::unique_ptr<QRhiTexture>>   textures_{};

    bool uploaded_{};
};

#endif //! RHI_MESH_H
