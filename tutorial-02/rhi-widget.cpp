#include "rhi-widget.h"

#include <QFile>

// clang-format off
// normalized device coordinates
static constexpr float vertices[] = {
    -1.0f, -1.0f,  /* bottom left  */  0.0f, 1.0f, /* top    left  */
    +1.0f, -1.0f,  /* bottom right */  1.0f, 1.0f, /* top    right */
    -1.0f, +1.0f,  /* top    left  */  0.0f, 0.0f, /* bottom left  */
    +1.0f, +1.0f,  /* top    right */  1.0f, 0.0f, /* bottom right */
};
// clang-format on

QShader LoadShader(const QString& name)
{
    QFile f(name);
    return f.open(QIODevice::ReadOnly) ? QShader::fromSerialized(f.readAll()) : QShader();
}

RhiWidget::RhiWidget() { cover_.load(":/images/cover.png"); }

void RhiWidget::initialize(QRhiCommandBuffer *cb)
{
    if (rhi_ != rhi()) {
        pipeline_.reset();
        rhi_ = rhi();
    }

    if (!pipeline_) {
        vbuf_.reset(rhi_->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(vertices)));
        vbuf_->create();

        ubuf_.reset(rhi_->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 64));
        ubuf_->create();

        sampler_.reset(rhi_->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                        QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge));
        sampler_->create();

        texture_.reset(rhi_->newTexture(QRhiTexture::RGBA8, cover_.size()));
        texture_->create();

        std::vector<QRhiShaderResourceBinding> bindings{};
        bindings.emplace_back(QRhiShaderResourceBinding::uniformBuffer(
            0, QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage,
            ubuf_.get()));
        bindings.emplace_back(QRhiShaderResourceBinding::sampledTexture(
            1, QRhiShaderResourceBinding::FragmentStage, texture_.get(), sampler_.get()));
        srb_.reset(rhi_->newShaderResourceBindings());
        srb_->setBindings(bindings.begin(), bindings.end());
        srb_->create();

        pipeline_.reset(rhi_->newGraphicsPipeline());
        pipeline_->setTopology(QRhiGraphicsPipeline::TriangleStrip);
        pipeline_->setShaderStages({
            { QRhiShaderStage::Vertex, LoadShader(":/vertex.vert.qsb") },
            { QRhiShaderStage::Fragment, LoadShader(":/fragment.frag.qsb") },
        });

        QRhiVertexInputLayout layout{};
        layout.setBindings({ 4 * sizeof(float) });
        layout.setAttributes({
            { 0, 0, QRhiVertexInputAttribute::Float2, 0 },
            { 0, 1, QRhiVertexInputAttribute::Float2, 2 * sizeof(float) },
        });
        pipeline_->setVertexInputLayout(layout);
        pipeline_->setShaderResourceBindings(srb_.get());
        pipeline_->setRenderPassDescriptor(renderTarget()->renderPassDescriptor());
        pipeline_->create();

        auto rub = rhi_->nextResourceUpdateBatch();
        rub->uploadStaticBuffer(vbuf_.get(), vertices);
        rub->uploadTexture(texture_.get(), cover_);
        cb->resourceUpdate(rub);
    }
}

void RhiWidget::render(QRhiCommandBuffer *cb)
{
    const auto rub  = rhi_->nextResourceUpdateBatch();
    const auto rtsz = renderTarget()->pixelSize();

    const auto scaled  = cover_.size().scaled(rtsz, Qt::KeepAspectRatio);
    float      scale_x = (static_cast<float>(scaled.width()) / static_cast<float>(rtsz.width()));
    float      scale_y = (static_cast<float>(scaled.height()) / static_cast<float>(rtsz.height()));
    mvp_.setToIdentity();
    mvp_.scale(scale_x, scale_y);
    rub->updateDynamicBuffer(ubuf_.get(), 0, 64, mvp_.constData());

    auto background = QColor::fromRgbF(0.0f, 0.0f, 0.0f, 1.0f);
    cb->beginPass(renderTarget(), background, { 1.0f, 0 }, rub);

    cb->setGraphicsPipeline(pipeline_.get());
    cb->setViewport({ 0, 0, static_cast<float>(rtsz.width()), static_cast<float>(rtsz.height()) });
    cb->setShaderResources();

    const QRhiCommandBuffer::VertexInput input{ vbuf_.get(), 0 };
    cb->setVertexInput(0, 1, &input);
    cb->draw(4);

    cb->endPass();
}
