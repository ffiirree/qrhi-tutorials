#ifndef RHI_WIDGET_H
#define RHI_WIDGET_H

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

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    QRhi *rhi_{};

    QMatrix4x4 mvp_{};

    // mouse
    QVector2D   last_pos_{};
    QQuaternion rotation_{};

    std::vector<std::unique_ptr<RenderItem>> items_{};
};

#endif //! RHI_WIDGET_H