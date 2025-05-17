#ifndef RHI_WIDGET_H
#define RHI_WIDGET_H

#include <QRhiWidget>
#include <rhi/qrhi.h>

class RhiWidget : public QRhiWidget
{
    Q_OBJECT

public:
    void initialize(QRhiCommandBuffer *) override;

    void render(QRhiCommandBuffer *cb) override;

private:
    QRhi *rhi_{};

    std::unique_ptr<QRhiGraphicsPipeline>       pipeline_{};
    std::unique_ptr<QRhiBuffer>                 vbuf_{}; // vertex buffer
    std::unique_ptr<QRhiShaderResourceBindings> srb_{};
};

#endif //! RHI_WIDGET_H