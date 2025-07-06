#ifndef RHI_RENDER_ITEM_H
#define RHI_RENDER_ITEM_H

#include <rhi/qrhi.h>

struct RenderItem
{
    virtual ~RenderItem() = default;

    virtual void create(QRhi *rhi, QRhiRenderTarget *rt)                   = 0;
    virtual void upload(QRhiResourceUpdateBatch *rub, const QMatrix4x4& mvp,
                        const std::vector<QMatrix4x4>&)                    = 0;
    virtual void draw(QRhiCommandBuffer *cb, const QRhiViewport& viewport) = 0;
};

#endif //! RHI_RENDER_ITEM_H
