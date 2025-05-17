#include "mesh.h"

#include <QFile>
#include <utility>

static QShader LoadShader(const QString& name)
{
    QFile f(name);
    return f.open(QIODevice::ReadOnly) ? QShader::fromSerialized(f.readAll()) : QShader();
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::vector<Material> materials)
    : vertices(std::move(vertices)), indices(std::move(indices)), materials(std::move(materials))
{}

void Mesh::initialize(QRhi *rhi, QRhiRenderTarget *rt, const std::map<std::string, QImage>& images)
{
    if (rhi_ != rhi) {
        pipeline_.reset();
        rhi_ = rhi;
    }

    if (pipeline_) return;

    vbuf_.reset(
        rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, vertices.size() * sizeof(Vertex)));
    vbuf_->create();

    ubuf_.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 64));
    ubuf_->create();

    sampler_.reset(rhi->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                   QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge));
    sampler_->create();

    std::vector<QRhiShaderResourceBinding> bindings{};
    bindings.emplace_back(QRhiShaderResourceBinding::uniformBuffer(
        0, QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage, ubuf_.get()));
    for (size_t i = 0; i < materials.size(); ++i) {
        textures_.emplace_back(rhi_->newTexture(QRhiTexture::RGBA8, images.at(materials[i].path).size()));
        textures_.back()->create();

        bindings.emplace_back(QRhiShaderResourceBinding::sampledTexture(
            static_cast<int>(i + 1), QRhiShaderResourceBinding::FragmentStage, textures_.back().get(),
            sampler_.get()));
    }

    srb_.reset(rhi->newShaderResourceBindings());
    srb_->setBindings(bindings.begin(), bindings.end());
    srb_->create();

    pipeline_.reset(rhi->newGraphicsPipeline());
    pipeline_->setTopology(QRhiGraphicsPipeline::Triangles);
    pipeline_->setShaderStages({
        { QRhiShaderStage::Vertex, LoadShader(":/vertex.vert.qsb") },
        { QRhiShaderStage::Fragment, LoadShader(":/fragment.frag.qsb") },
    });

    QRhiVertexInputLayout layout{};
    layout.setBindings({ 8 * sizeof(float) });
    layout.setAttributes({
        { 0, 0, QRhiVertexInputAttribute::Float3, 0 },
        { 0, 1, QRhiVertexInputAttribute::Float3, 3 * sizeof(float) },
        { 0, 2, QRhiVertexInputAttribute::Float2, 6 * sizeof(float) },
    });
    pipeline_->setVertexInputLayout(layout);
    pipeline_->setShaderResourceBindings(srb_.get());
    pipeline_->setRenderPassDescriptor(rt->renderPassDescriptor());
    pipeline_->setDepthTest(true);
    pipeline_->setDepthWrite(true);
    pipeline_->setCullMode(QRhiGraphicsPipeline::Back);
    pipeline_->create();
}

void Mesh::upload(QRhiResourceUpdateBatch *rub, const QMatrix4x4& mvp,
                  const std::map<std::string, QImage>& images)
{
    if (!uploaded_) {
        for (size_t i = 0; i < materials.size(); ++i) {
            rub->uploadTexture(textures_[i].get(), images.at(materials[i].path));
        }

        rub->uploadStaticBuffer(vbuf_.get(), vertices.data());
        uploaded_ = true;
    }

    rub->updateDynamicBuffer(ubuf_.get(), 0, 64, mvp.constData());
}

void Mesh::draw(QRhiCommandBuffer *cb, const QRhiViewport& viewport)
{
    cb->setGraphicsPipeline(pipeline_.get());
    cb->setViewport(viewport);
    cb->setShaderResources();

    const QRhiCommandBuffer::VertexInput input{ vbuf_.get(), 0 };
    cb->setVertexInput(0, 1, &input);
    cb->draw(vertices.size());
}