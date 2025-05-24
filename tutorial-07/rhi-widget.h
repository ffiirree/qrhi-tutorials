#ifndef RHI_WIDGET_H
#define RHI_WIDGET_H

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

private:
    QRhi *rhi_{};

    std::unique_ptr<QRhiGraphicsPipeline>       pipeline_{};
    std::unique_ptr<QRhiBuffer>                 vbuf_{};
    std::unique_ptr<QRhiBuffer>                 mvp_buf_{};
    std::unique_ptr<QRhiBuffer>                 light_buf_{};
    std::unique_ptr<QRhiBuffer>                 material_buf_{};
    std::unique_ptr<QRhiBuffer>                 camera_buf_{};
    std::unique_ptr<QRhiSampler>                sampler_{};
    std::unique_ptr<QRhiShaderResourceBindings> srb_{};
    std::unique_ptr<QRhiTexture>                texture_{};

    QMatrix4x4 model_{};
    QMatrix4x4 view_{};
    QMatrix4x4 projection_{};

    QVector3D camera_pos_{ 0.0f, 0.0f, -8.0f };

    // mouse
    QVector2D   last_pos_{};
    QQuaternion rotation_{};

    // lighting
    struct lighting_t
    {
        QVector3D position{ 0.0f, 4.0f, 0.0f };
        float     ambient[3]{ 0.2f, 0.2f, 0.2f };
        float     diffuse[3]{ 0.5f, 0.5f, 0.5f };
        float     specular[3]{ 1.0f, 1.0f, 1.0f };
    } light_{};

    // Material
    struct material_t
    {
        float ambient[3]{ 1.0f, 0.5f, 0.31f };
        float diffuse[3]{ 1.0f, 0.5f, 0.31f };
        float specular[3]{ 0.5f, 0.5f, 0.5f };
        float shininess{ 32.0f };
    } material_{};
};

#endif //! RHI_WIDGET_H