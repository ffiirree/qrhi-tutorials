#ifndef RHI_WIDGET_H
#define RHI_WIDGET_H

#include "animation.h"
#include "animator.h"
#include "render-item.h"

#include <QRhiWidget>
#include <rhi/qrhi.h>

class RhiWidget : public QRhiWidget
{
    Q_OBJECT

public:
    RhiWidget();

    void initialize(QRhiCommandBuffer *) override;

    void render(QRhiCommandBuffer *cb) override;

    void load(const QString& model);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    QRhi *rhi_{};

    QMatrix4x4 mvp_{};

    double last_ts_{};

    // mouse
    QVector2D   last_pos_{};
    QQuaternion rotation_{};
    float       scale_{ 1.0f };

    std::vector<std::unique_ptr<RenderItem>> items_{};
    std::vector<std::unique_ptr<Animation>>  animations_{};
    std::vector<std::unique_ptr<Animator>>   animators_{};
};

#endif //! RHI_WIDGET_H