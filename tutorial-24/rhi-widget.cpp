#include "rhi-widget.h"

#include <QFile>
#include <QMouseEvent>

// clang-format off
// normalized device coordinates
static constexpr float vertices[] = {
     10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
    -10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
    -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,

     10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
    -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
     10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f
};
// clang-format on

QShader LoadShader(const QString& name)
{
    QFile f(name);
    return f.open(QIODevice::ReadOnly) ? QShader::fromSerialized(f.readAll()) : QShader();
}

RhiWidget::RhiWidget()
{
    wood_.load(":/images/wood.png");
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

        mvp_buf_.reset(rhi_->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 64 * 3));
        mvp_buf_->create();

        light_buf_.reset(rhi_->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 4 * 4 * 4));
        light_buf_->create();

        material_buf_.reset(rhi_->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 16));
        material_buf_->create();

        camera_buf_.reset(rhi_->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 4 * 4));
        camera_buf_->create();

        sampler_.reset(rhi_->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                        QRhiSampler::Repeat, QRhiSampler::Repeat));
        sampler_->create();

        wood_texture_.reset(rhi_->newTexture(QRhiTexture::RGBA8, wood_.size()));
        wood_texture_->create();

        srb_.reset(rhi_->newShaderResourceBindings());
        srb_->setBindings({
            QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage,
                                                     mvp_buf_.get()),
            QRhiShaderResourceBinding::uniformBuffer(1, QRhiShaderResourceBinding::FragmentStage,
                                                     light_buf_.get()),
            QRhiShaderResourceBinding::uniformBuffer(2, QRhiShaderResourceBinding::FragmentStage,
                                                     material_buf_.get()),
            QRhiShaderResourceBinding::uniformBuffer(3, QRhiShaderResourceBinding::FragmentStage,
                                                     camera_buf_.get()),
            QRhiShaderResourceBinding::sampledTexture(4, QRhiShaderResourceBinding::FragmentStage,
                                                      wood_texture_.get(), sampler_.get()),
        });
        srb_->create();

        pipeline_.reset(rhi_->newGraphicsPipeline());
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
        pipeline_->setRenderPassDescriptor(renderTarget()->renderPassDescriptor());
//        pipeline_->setDepthTest(true);
//        pipeline_->setDepthWrite(true);
//        pipeline_->setCullMode(QRhiGraphicsPipeline::Back);
        pipeline_->create();

        auto rub = rhi_->nextResourceUpdateBatch();
        rub->uploadStaticBuffer(vbuf_.get(), vertices);
        rub->updateDynamicBuffer(light_buf_.get(), 0, 12, &light_.position);
        rub->updateDynamicBuffer(light_buf_.get(), 16, 12, light_.ambient);
        rub->updateDynamicBuffer(light_buf_.get(), 32, 12, light_.diffuse);
        rub->updateDynamicBuffer(light_buf_.get(), 48, 12, light_.specular);

        rub->updateDynamicBuffer(material_buf_.get(), 0, 12, material_.specular);
        rub->updateDynamicBuffer(material_buf_.get(), 12, 4, &material_.shininess);

        rub->updateDynamicBuffer(camera_buf_.get(), 0, 12, &camera_pos_);

        rub->uploadTexture(wood_texture_.get(), wood_);
        cb->resourceUpdate(rub);
    }
}

void RhiWidget::render(QRhiCommandBuffer *cb)
{
    const auto rub  = rhi_->nextResourceUpdateBatch();
    const auto rtsz = renderTarget()->pixelSize();

    model_.setToIdentity();
    model_.rotate(rotation_);

    view_.setToIdentity();
    view_.translate(camera_pos_);

    projection_.setToIdentity();
    projection_.perspective(45.0f, rtsz.width() / (float)rtsz.height(), 0.1f, 100.0f);

    rub->updateDynamicBuffer(mvp_buf_.get(), 0, 64, model_.constData());
    rub->updateDynamicBuffer(mvp_buf_.get(), 64, 64, view_.constData());
    rub->updateDynamicBuffer(mvp_buf_.get(), 128, 64, projection_.constData());

    cb->beginPass(renderTarget(), Qt::black, { 1.0f, 0 }, rub);

    cb->setGraphicsPipeline(pipeline_.get());
    cb->setViewport({ 0, 0, static_cast<float>(rtsz.width()), static_cast<float>(rtsz.height()) });
    cb->setShaderResources();

    const QRhiCommandBuffer::VertexInput input{ vbuf_.get(), 0 };
    cb->setVertexInput(0, 1, &input);
    cb->draw(6);

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
