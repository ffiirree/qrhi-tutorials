#include "rhi-widget.h"

#include <QFile>
#include <QMouseEvent>

// clang-format off
// normalized device coordinates
static constexpr float vertices[] = {
    -0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
     0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
    -0.05f, -0.05f,  0.0f, 0.0f, 1.0f,

    -0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
     0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
     0.05f,  0.05f,  0.0f, 1.0f, 1.0f
};
// clang-format on

QShader LoadShader(const QString& name)
{
    QFile f(name);
    return f.open(QIODevice::ReadOnly) ? QShader::fromSerialized(f.readAll()) : QShader();
}

RhiWidget::RhiWidget()
{
    for (int i = -10; i < 10; i += 2) {
        for (int j = -10; j < 10; j += 2) {
            instance_offsets_.emplace_back(i * 0.1f + 0.1, j * 0.1f + 0.1);
        }
    }
}

void RhiWidget::initialize(QRhiCommandBuffer *cb)
{
    if (rhi_ != rhi()) {
        pipeline_.reset();
        rhi_ = rhi();
    }

    if (!pipeline_) {
        vertex_buf_.reset(
            rhi_->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(vertices)));
        vertex_buf_->create();

        instancing_buf_.reset(rhi_->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer,
                                              instance_offsets_.size() * sizeof(QVector2D)));
        instancing_buf_->create();

        srb_.reset(rhi_->newShaderResourceBindings());
        srb_->create();

        pipeline_.reset(rhi_->newGraphicsPipeline());
        pipeline_->setTopology(QRhiGraphicsPipeline::Triangles);
        pipeline_->setShaderStages({
            { QRhiShaderStage::Vertex, LoadShader(":/vertex.vert.qsb") },
            { QRhiShaderStage::Fragment, LoadShader(":/fragment.frag.qsb") },
        });

        QRhiVertexInputLayout layout{};
        layout.setBindings({
            { 5 * sizeof(float) },
            { 2 * sizeof(float), QRhiVertexInputBinding::PerInstance },
        });
        layout.setAttributes({
            { 0, 0, QRhiVertexInputAttribute::Float2, 0 },
            { 0, 1, QRhiVertexInputAttribute::Float3, 2 * sizeof(float) },
            { 1, 2, QRhiVertexInputAttribute::Float2, 0 },
        });
        pipeline_->setVertexInputLayout(layout);
        pipeline_->setShaderResourceBindings(srb_.get());
        pipeline_->setRenderPassDescriptor(renderTarget()->renderPassDescriptor());
        pipeline_->create();

        auto rub = rhi_->nextResourceUpdateBatch();
        rub->uploadStaticBuffer(vertex_buf_.get(), vertices);
        rub->uploadStaticBuffer(instancing_buf_.get(), instance_offsets_.data());
        cb->resourceUpdate(rub);
    }
}

void RhiWidget::render(QRhiCommandBuffer *cb)
{
    const auto rub  = rhi_->nextResourceUpdateBatch();
    const auto rtsz = renderTarget()->pixelSize();

    cb->beginPass(renderTarget(), Qt::black, { 1.0f, 0 }, rub);

    cb->setGraphicsPipeline(pipeline_.get());
    cb->setViewport({ 0, 0, static_cast<float>(rtsz.width()), static_cast<float>(rtsz.height()) });
    cb->setShaderResources();

    const QRhiCommandBuffer::VertexInput input[] = {
        { vertex_buf_.get(), 0 },
        { instancing_buf_.get(), 0 },
    };
    cb->setVertexInput(0, 2, input);
    cb->draw(6, instance_offsets_.size());

    cb->endPass();
}
