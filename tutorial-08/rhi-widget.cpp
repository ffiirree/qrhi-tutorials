#include "rhi-widget.h"

#include <QFile>
#include <QMimeData>
#include <QMouseEvent>

RhiWidget::RhiWidget() { setAcceptDrops(true); }

void RhiWidget::initialize(QRhiCommandBuffer *cb)
{
    if (rhi_ != rhi()) {
        rhi_ = rhi();
    }
}

void RhiWidget::render(QRhiCommandBuffer *cb)
{
    const auto rub  = rhi_->nextResourceUpdateBatch();
    const auto rtsz = renderTarget()->pixelSize();

    model_.initialize(rhi_, renderTarget());

    mvp_.setToIdentity();
    mvp_.perspective(45.0f, rtsz.width() / (float)rtsz.height(), 0.01f, 1000.0f);
    mvp_.translate(0, 0, -8.0);
    mvp_.rotate(rotation_);

    model_.upload(rub, mvp_);

    cb->beginPass(renderTarget(), Qt::black, { 1.0f, 0 }, rub);

    model_.draw(cb, { 0, 0, static_cast<float>(rtsz.width()), static_cast<float>(rtsz.height()) });

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

void RhiWidget::dragEnterEvent(QDragEnterEvent *event)
{
    const auto mimedata = event->mimeData();
    if (mimedata->hasUrls()) {
        event->acceptProposedAction();
    }
    else {
        event->ignore();
    }
}

void RhiWidget::dropEvent(QDropEvent *event)
{
    const auto mimedata = event->mimeData();

    if (mimedata->hasUrls()) {
        model_.load(mimedata->urls()[0].toLocalFile());

        update();
    }
}
