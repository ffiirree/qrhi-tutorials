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

private:
    QRhi *rhi_{};

    std::unique_ptr<QRhiGraphicsPipeline>       pipeline_{};
    std::unique_ptr<QRhiBuffer>                 vertex_buf_{};
    std::unique_ptr<QRhiBuffer>                 instancing_buf_{};
    std::unique_ptr<QRhiShaderResourceBindings> srb_{};

    std::vector<QVector2D> instance_offsets_{};
};

#endif //! RHI_WIDGET_H