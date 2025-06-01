#include "rhi-widget.h"

#include "camera.h"

#include <QFile>
#include <QMouseEvent>

// clang-format off
// normalized device coordinates
static constexpr float vertices[] = {
    -1.0f, -1.0f, -1.0f,  0.0f, 1.0f,  // -X
    -1.0f, -1.0f,  1.0f,  1.0f, 1.0f,
    -1.0f,  1.0f,  1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f,  1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, -1.0f,  0.0f, 0.0f,
    -1.0f, -1.0f, -1.0f,  0.0f, 1.0f,

    -1.0f, -1.0f, -1.0f,  1.0f, 1.0f,  // -Z
     1.0f,  1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f, -1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,  1.0f, 1.0f,
    -1.0f,  1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f, -1.0f,  0.0f, 0.0f,

    -1.0f, -1.0f, -1.0f,  1.0f, 0.0f,  // -Y
     1.0f, -1.0f, -1.0f,  1.0f, 1.0f,
     1.0f, -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,  1.0f, 0.0f,
     1.0f, -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  1.0f,  0.0f, 0.0f,

    -1.0f,  1.0f, -1.0f,  1.0f, 0.0f,  // +Y
    -1.0f,  1.0f,  1.0f,  0.0f, 0.0f,
     1.0f,  1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f,  1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f,  0.0f, 1.0f,
     1.0f,  1.0f, -1.0f,  1.0f, 1.0f,

     1.0f,  1.0f, -1.0f,  1.0f, 0.0f,  // +X
     1.0f,  1.0f,  1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f, -1.0f,  1.0f, 1.0f,
     1.0f,  1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  1.0f,  0.0f, 0.0f,  // +Z
    -1.0f, -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f,  1.0f,  1.0f,  1.0f, 0.0f,
    -1.0f, -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f,  1.0f, 1.0f,
     1.0f,  1.0f,  1.0f,  1.0f, 0.0f,
};
// clang-format on

QShader LoadShader(const QString& name)
{
    QFile f(name);
    return f.open(QIODevice::ReadOnly) ? QShader::fromSerialized(f.readAll()) : QShader();
}

RhiWidget::RhiWidget()
{
    setSampleCount(4);
    cover_.load(":/images/cover.png");
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

        ubuf_.reset(rhi_->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 64));
        ubuf_->create();

        sampler_.reset(rhi_->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                        QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge));
        sampler_->create();

        texture_.reset(rhi_->newTexture(QRhiTexture::RGBA8, cover_.size()));
        texture_->create();

        srb_.reset(rhi_->newShaderResourceBindings());
        srb_->setBindings({
            QRhiShaderResourceBinding::uniformBuffer(
                0, QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage,
                ubuf_.get()),
            QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage,
                                                      texture_.get(), sampler_.get()),
        });
        srb_->create();

        pipeline_.reset(rhi_->newGraphicsPipeline());
        pipeline_->setTopology(QRhiGraphicsPipeline::Triangles);
        pipeline_->setShaderStages({
            { QRhiShaderStage::Vertex, LoadShader(":/vertex.vert.qsb") },
            { QRhiShaderStage::Fragment, LoadShader(":/fragment.frag.qsb") },
        });

        QRhiVertexInputLayout layout{};
        layout.setBindings({ 5 * sizeof(float) });
        layout.setAttributes({
            { 0, 0, QRhiVertexInputAttribute::Float3, 0 },
            { 0, 1, QRhiVertexInputAttribute::Float2, 3 * sizeof(float) },
        });
        pipeline_->setVertexInputLayout(layout);
        pipeline_->setShaderResourceBindings(srb_.get());
        pipeline_->setRenderPassDescriptor(renderTarget()->renderPassDescriptor());
        pipeline_->setDepthTest(true);
        pipeline_->setDepthWrite(true);
        pipeline_->setCullMode(QRhiGraphicsPipeline::Back);
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

    model_.setToIdentity();
    model_.rotate(rotation_);

    view_ = Camera::lookAt({ 0, 0, 8.0 }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });

    projection_.setToIdentity();
    projection_.perspective(45.0f, rtsz.width() / (float)rtsz.height(), 0.01f, 1000.0f);

    rub->updateDynamicBuffer(ubuf_.get(), 0, 64, (projection_ * view_ * model_).constData());

    cb->beginPass(renderTarget(), Qt::black, { 1.0f, 0 }, rub);

    cb->setGraphicsPipeline(pipeline_.get());
    cb->setViewport({ 0, 0, static_cast<float>(rtsz.width()), static_cast<float>(rtsz.height()) });
    cb->setShaderResources();

    const QRhiCommandBuffer::VertexInput input{ vbuf_.get(), 0 };
    cb->setVertexInput(0, 1, &input);
    cb->draw(36);

    cb->endPass();
}

void RhiWidget::mousePressEvent(QMouseEvent *event)
{
    last_pos_ = QVector2D(event->position());
    QWidget::mousePressEvent(event);
}

void RhiWidget::mouseMoveEvent(QMouseEvent *event)
{
    const auto diff = QVector2D(event->position()) - last_pos_;
    last_pos_       = QVector2D(event->position());

    const auto axis = QVector3D(diff.y(), diff.x(), 0.0).normalized();
    rotation_       = QQuaternion::fromAxisAndAngle(axis, diff.length()) * rotation_;

    update();

    QWidget::mouseMoveEvent(event);
}
