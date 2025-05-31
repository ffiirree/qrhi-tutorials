#include "mesh.h"

#include <QFile>
#include <utility>

static QShader LoadShader(const QString& name)
{
    QFile f(name);
    return f.open(QIODevice::ReadOnly) ? QShader::fromSerialized(f.readAll()) : QShader();
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::vector<Material> materials,
           aiMatrix4x4 transform)
    : vertices(std::move(vertices)), indices(std::move(indices)), materials(std::move(materials))
{
    view_ = QMatrix4x4(transform.a1, transform.a2, transform.a3, transform.a4, transform.b1, transform.b2,
                       transform.b3, transform.b4, transform.c1, transform.c2, transform.c3, transform.c4,
                       transform.d1, transform.d2, transform.d3, transform.d4);
}

void Mesh::create(QRhi *rhi, QRhiRenderTarget *rt)
{
    if (rhi_ != rhi) {
        pipeline_.reset();
        rhi_ = rhi;
    }

    if (!pipeline_) {
        vbuf_.reset(rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer,
                                   vertices.size() * sizeof(Vertex)));
        vbuf_->create();

        ibuf_.reset(rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::IndexBuffer,
                                   indices.size() * sizeof(uint32_t)));
        ibuf_->create();

        ubuf_.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 64));
        ubuf_->create();

        sampler_.reset(rhi->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                       QRhiSampler::Repeat, QRhiSampler::Repeat));
        sampler_->create();

        std::vector<QRhiShaderResourceBinding> bindings{};
        bindings.emplace_back(QRhiShaderResourceBinding::uniformBuffer(
            0, QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage,
            ubuf_.get()));
        for (size_t i = 0; i < materials.size(); ++i) {
            bindings.emplace_back(QRhiShaderResourceBinding::sampledTexture(
                static_cast<int>(i + 1), QRhiShaderResourceBinding::FragmentStage,
                materials[i].texture.get(), sampler_.get()));
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
        // pipeline_->setPolygonMode(QRhiGraphicsPipeline::Line);
        pipeline_->create();
    }
}

void Mesh::upload(QRhiResourceUpdateBatch *rub, const QMatrix4x4& mvp)
{
    if (!uploaded_) {
        rub->uploadStaticBuffer(vbuf_.get(), vertices.data());
        rub->uploadStaticBuffer(ibuf_.get(), indices.data());
        uploaded_ = true;
    }

    rub->updateDynamicBuffer(ubuf_.get(), 0, 64, (mvp * view_).constData());
}

void Mesh::draw(QRhiCommandBuffer *cb, const QRhiViewport& viewport)
{
    cb->setGraphicsPipeline(pipeline_.get());
    cb->setViewport(viewport);
    cb->setShaderResources();

    const QRhiCommandBuffer::VertexInput input{ vbuf_.get(), 0 };
    cb->setVertexInput(0, 1, &input, ibuf_.get(), 0, QRhiCommandBuffer::IndexUInt32);
    cb->drawIndexed(indices.size());
}