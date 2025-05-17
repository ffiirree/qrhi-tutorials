#include "rhi-widget.h"

#include <QFile>

// clang-format off
// normalized device coordinates
static constexpr float vertices[] = {
     0.0f,   0.5f,     1.0f, 0.0f, 0.0f,
    -0.5f,  -0.5f,     0.0f, 1.0f, 0.0f,
     0.5f,  -0.5f,     0.0f, 0.0f, 1.0f,
};
// clang-format on

QShader LoadShader(const QString& name)
{
    QFile f(name);
    return f.open(QIODevice::ReadOnly) ? QShader::fromSerialized(f.readAll()) : QShader();
}

void RhiWidget::initialize(QRhiCommandBuffer *cb)
{
    if (rhi_ != rhi()) {
        pipeline_.reset();
        rhi_ = rhi();
    }

    if (!pipeline_) {
        vbuf_.reset(rhi_->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(vertices)));
        vbuf_->create();

        srb_.reset(rhi_->newShaderResourceBindings());
        srb_->create();

        pipeline_.reset(rhi_->newGraphicsPipeline());
        pipeline_->setShaderStages({
            { QRhiShaderStage::Vertex, LoadShader(":/vertex.vert.qsb") },
            { QRhiShaderStage::Fragment, LoadShader(":/fragment.frag.qsb") },
        });

        QRhiVertexInputLayout layout{};
        layout.setBindings({ 5 * sizeof(float) });
        layout.setAttributes({
            { 0, 0, QRhiVertexInputAttribute::Float2, 0 },
            { 0, 1, QRhiVertexInputAttribute::Float3, 2 * sizeof(float) },
        });
        pipeline_->setVertexInputLayout(layout);
        pipeline_->setShaderResourceBindings(srb_.get());
        pipeline_->setRenderPassDescriptor(renderTarget()->renderPassDescriptor());
        pipeline_->create();

        auto rub = rhi_->nextResourceUpdateBatch();
        rub->uploadStaticBuffer(vbuf_.get(), vertices);
        cb->resourceUpdate(rub);
    }
}

void RhiWidget::render(QRhiCommandBuffer *cb)
{
    const auto rub  = rhi_->nextResourceUpdateBatch();
    const auto rtsz = renderTarget()->pixelSize();

    auto background = QColor::fromRgbF(0.0f, 0.0f, 0.0f, 1.0f);
    cb->beginPass(renderTarget(), background, { 1.0f, 0 }, rub);

    cb->setGraphicsPipeline(pipeline_.get());
    cb->setViewport({ 0, 0, static_cast<float>(rtsz.width()), static_cast<float>(rtsz.height()) });
    cb->setShaderResources();

    const QRhiCommandBuffer::VertexInput input{ vbuf_.get(), 0 };
    cb->setVertexInput(0, 1, &input);
    cb->draw(3);

    cb->endPass();
}
