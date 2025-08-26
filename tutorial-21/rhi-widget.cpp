#include "rhi-widget.h"

#include <numbers>
#include <QFile>
#include <QMouseEvent>

QShader LoadShader(const QString& name)
{
    QFile f(name);
    return f.open(QIODevice::ReadOnly) ? QShader::fromSerialized(f.readAll()) : QShader();
}
RhiWidget::RhiWidget()
{
    setSampleCount(4);

    for (int i = -10; i < 10; i += 2) {
        for (int j = -10; j < 10; j += 2) {
            instance_offsets_.emplace_back(i * 4.0f + 2.0f, j * 4.0f + 2.0f);
        }
    }

    constexpr int N = 30;

    float dYaw   = 2.0f * std::numbers::pi / float(N);
    float dPitch = std::numbers::pi / float(N);
    for (int i = 0; i <= N; ++i) {
        float yaw = i * dYaw;
        for (int j = 0; j <= N; ++j) {
            float pitch = -std::numbers::pi / 2 - j * dPitch; // 为了正确计算UV坐标，维度从南极开始计算
            float x     = std::cos(pitch) * std::cos(yaw);
            float y     = std::cos(pitch) * std::sin(yaw);
            float z     = std::sin(pitch);

            vertices_.emplace_back(QVector3D{ x, y, z }, QVector3D{ x, y, z }.normalized());
        }
    }

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            indices_.push_back(i * (N + 1) + j);
            indices_.push_back((i + 1) * (N + 1) + j);
            indices_.push_back(i * (N + 1) + j + 1);
            indices_.push_back(i * (N + 1) + j + 1);
            indices_.push_back((i + 1) * (N + 1) + j);
            indices_.push_back((i + 1) * (N + 1) + j + 1);
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
        vertex_buf_.reset(rhi_->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer,
                                          vertices_.size() * sizeof(Vertex)));
        vertex_buf_->create();

        instancing_buf_.reset(rhi_->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer,
                                              instance_offsets_.size() * sizeof(QVector2D)));
        instancing_buf_->create();

        index_buf_.reset(rhi_->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::IndexBuffer,
                                         static_cast<quint32>(indices_.size() * sizeof(uint32_t))));
        index_buf_->create();

        ubuf_.reset(rhi_->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 64 * 3));
        ubuf_->create();

        std::vector<QRhiShaderResourceBinding> bindings{};
        bindings.emplace_back(QRhiShaderResourceBinding::uniformBuffer(
            0, QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage,
            ubuf_.get()));

        srb_.reset(rhi_->newShaderResourceBindings());
        srb_->setBindings(bindings.begin(), bindings.end());
        srb_->create();

        pipeline_.reset(rhi_->newGraphicsPipeline());
        pipeline_->setTopology(QRhiGraphicsPipeline::Triangles);
        pipeline_->setShaderStages({
            { QRhiShaderStage::Vertex, LoadShader(":/vertex.vert.qsb") },
            { QRhiShaderStage::Fragment, LoadShader(":/fragment.frag.qsb") },
        });

        QRhiVertexInputLayout layout{};
        layout.setBindings({
            { 6 * sizeof(float) },
            { 2 * sizeof(float), QRhiVertexInputBinding::PerInstance  },
        });
        layout.setAttributes({
            { 0, 0, QRhiVertexInputAttribute::Float3, 0 },
            { 0, 1, QRhiVertexInputAttribute::Float3, 3 * sizeof(float) },
            { 1, 2, QRhiVertexInputAttribute::Float2, 0 },
        });
        pipeline_->setVertexInputLayout(layout);
        pipeline_->setShaderResourceBindings(srb_.get());
        pipeline_->setRenderPassDescriptor(renderTarget()->renderPassDescriptor());
        pipeline_->setDepthTest(true);
        pipeline_->setDepthWrite(true);
        pipeline_->setCullMode(QRhiGraphicsPipeline::Back);
        pipeline_->create();

        auto rub = rhi_->nextResourceUpdateBatch();
        rub->uploadStaticBuffer(vertex_buf_.get(), vertices_.data());
        rub->uploadStaticBuffer(instancing_buf_.get(), instance_offsets_.data());
        rub->uploadStaticBuffer(index_buf_.get(), indices_.data());
        cb->resourceUpdate(rub);
    }
}

void RhiWidget::render(QRhiCommandBuffer *cb)
{
    const auto rub  = rhi_->nextResourceUpdateBatch();
    const auto rtsz = renderTarget()->pixelSize();

    model_.setToIdentity();
    model_.rotate(rotation_);
    model_.scale(scale_);

    view_.setToIdentity();
    view_.translate({ 0.0f, 0.0f, -8.0f });

    projection_.setToIdentity();
    projection_.perspective(45.0f, rtsz.width() / (float)rtsz.height(), 0.01f, 1000.0f);

    rub->updateDynamicBuffer(ubuf_.get(), 0, 64, model_.constData());
    rub->updateDynamicBuffer(ubuf_.get(), 64, 64, view_.constData());
    rub->updateDynamicBuffer(ubuf_.get(), 128, 64, projection_.constData());

    cb->beginPass(renderTarget(), Qt::black, { 1.0f, 0 }, rub);

    cb->setGraphicsPipeline(pipeline_.get());
    cb->setViewport({ 0, 0, static_cast<float>(rtsz.width()), static_cast<float>(rtsz.height()) });
    cb->setShaderResources();

    const QRhiCommandBuffer::VertexInput input[] = {
        { vertex_buf_.get(), 0 },
        { instancing_buf_.get(), 0 },
    };
    cb->setVertexInput(0, 2, input, index_buf_.get(), 0, QRhiCommandBuffer::IndexUInt32);
    cb->drawIndexed(indices_.size(), 100);

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

    const auto axis = QVector3D(diff.y() / 4, diff.x() / 4, 0.0).normalized();
    rotation_       = QQuaternion::fromAxisAndAngle(axis, diff.length()) * rotation_;

    update();

    QWidget::mouseMoveEvent(event);
}

void RhiWidget::wheelEvent(QWheelEvent *event)
{
    scale_ *= event->angleDelta().y() < 0 ? 0.95f : 1.05f;

    update();

    QWidget::wheelEvent(event);
}
