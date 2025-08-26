#ifndef RHI_WIDGET_H
#define RHI_WIDGET_H

#include <QRhiWidget>
#include <rhi/qrhi.h>

struct Vertex
{
    QVector3D position{};
    QVector3D normal{};
};

class RhiWidget final : public QRhiWidget
{
    Q_OBJECT

public:
    RhiWidget();

    void initialize(QRhiCommandBuffer *) override;

    void render(QRhiCommandBuffer *cb) override;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    QRhi *rhi_{};

    std::unique_ptr<QRhiGraphicsPipeline>       pipeline_{};
    std::unique_ptr<QRhiBuffer>                 vertex_buf_{};
    std::unique_ptr<QRhiBuffer>                 index_buf_{};
    std::unique_ptr<QRhiBuffer>                 ubuf_{};
    std::unique_ptr<QRhiBuffer>                 instancing_buf_{};
    std::unique_ptr<QRhiShaderResourceBindings> srb_{};

    std::vector<Vertex>    vertices_{};
    std::vector<uint32_t>  indices_{};

    std::vector<QVector2D> instance_offsets_{};

    QMatrix4x4 model_{};
    QMatrix4x4 view_{};
    QMatrix4x4 projection_{};

    // mouse
    QVector2D   last_pos_{};
    QQuaternion rotation_{};
    float       scale_{ 0.05f };
};

#endif //! RHI_WIDGET_H