#include "rhi-widget.h"

#include "model.h"

#include <QFile>
#include <QMimeData>
#include <QMouseEvent>

RhiWidget::RhiWidget()
{
    setAcceptDrops(true);
    setSampleCount(4);
}

void RhiWidget::initialize(QRhiCommandBuffer *)
{
    if (rhi_ != rhi()) {
        rhi_ = rhi();
    }
}

inline auto ns()
{
    using namespace std::chrono;
    return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch());
}

void RhiWidget::render(QRhiCommandBuffer *cb)
{
    const auto ts = ns().count() / 1000000000.0;
    if (last_ts_ == 0.0) last_ts_ = ts;

    const auto rub  = rhi_->nextResourceUpdateBatch();
    const auto rtsz = renderTarget()->pixelSize();

    mvp_.setToIdentity();
    mvp_.perspective(45.0f, rtsz.width() / (float)rtsz.height(), 0.01f, 3000.0f);
    mvp_.translate({ 0.0f, -2.25f, -8.0f });
    mvp_.rotate(rotation_);
    mvp_.scale(scale_);

    for (auto& item : items_) {

        animators_[0]->update(ts - last_ts_);
        const auto transforms = animators_[0]->bone_matrices();

        item->create(rhi_, renderTarget());
        item->upload(rub, mvp_, transforms);
    }

    last_ts_ = ts;

    cb->beginPass(renderTarget(), Qt::black, { 1.0f, 0 }, rub);

    for (auto& item : items_) {
        item->draw(cb, { 0, 0, static_cast<float>(rtsz.width()), static_cast<float>(rtsz.height()) });
    }

    cb->endPass();

    update();
}

void RhiWidget::load(const QString& path)
{
    items_.clear();
    animations_.clear();
    animators_.clear();

    items_.emplace_back(std::make_unique<Model>(path));
    animations_.emplace_back(
        std::make_unique<Animation>(path.toStdString(), dynamic_cast<Model *>(items_.back().get())));

    animators_.emplace_back(std::make_unique<Animator>(animations_.back().get()));
    update();
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
    items_.clear();
    animations_.clear();
    animators_.clear();

    for (const auto mimedata = event->mimeData(); auto& url : mimedata->urls()) {
        items_.emplace_back(std::make_unique<Model>(url.toLocalFile()));
        animations_.emplace_back(std::make_unique<Animation>(url.toLocalFile().toStdString(),
                                                             dynamic_cast<Model *>(items_.back().get())));

        animators_.emplace_back(std::make_unique<Animator>(animations_.back().get()));
    }

    update();
}
