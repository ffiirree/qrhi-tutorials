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
    std::unique_ptr<QRhiBuffer>                 vbuf_{};
    std::unique_ptr<QRhiBuffer>                 ubuf_{};
    std::unique_ptr<QRhiSampler>                sampler_{};
    std::unique_ptr<QRhiShaderResourceBindings> srb_{};
    std::unique_ptr<QRhiTexture>                texture_{};

    QMatrix4x4 mvp_{};

    QImage cover_{};
};

#endif //! RHI_WIDGET_H