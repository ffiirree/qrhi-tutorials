#include "rhi-widget.h"

#include <QFile>

// clang-format off
// normalized device coordinates
static constexpr float vertices[] = {
     0.0f,   0.5f,     1.0f, 0.0f, 0.0f,
    -0.5f,  -0.5f,     0.0f, 1.0f, 0.0f,
     0.5f,  -0.5f,     0.0f, 0.0f, 1.0f,
};
// clang-format on

QShader LoadShader(const QString& name)
{
    QFile f(name);
    return f.open(QIODevice::ReadOnly) ? QShader::fromSerialized(f.readAll()) : QShader();
}

// Called when the widget is initialized for the first time, when the associated texture's size, format, or
// sample count changes, or when the QRhi and texture change for any reason. The function is expected to
// maintain (create if not yet created, adjust and rebuild if the size has changed) the graphics resources
// used by the rendering code in render().
//
// To query the QRhi, QRhiTexture, and other related objects, call rhi(), colorTexture(),
// depthStencilBuffer(), and renderTarget().
//
// When the widget size changes, the QRhi object, the color buffer texture, and the depth stencil buffer
// objects are all the same instances (so the getters return the same pointers) as before, but the color and
// depth/stencil buffers will likely have been rebuilt, meaning the size and the underlying native texture
// resource may be different than in the last invocation.
//
// Reimplementations should also be prepared that the QRhi object and the color buffer texture may change
// between invocations of this function. One special case where the objects will be different is when
// performing a grabFramebuffer() with a widget that is not yet shown, and then making the widget visible
// on-screen within a top-level widget. There the grab will happen with a dedicated QRhi that is then
// replaced with the top-level window's associated QRhi in subsequent initialize() and render() invocations.
// Another, more common case is when the widget is reparented so that it belongs to a new top-level window.
// In this case the QRhi and all related resources managed by the QRhiWidget will be different instances
// than before in the subsequent call to this function. Is is then important that all existing QRhi
// resources previously created by the subclass are destroyed because they belong to the previous QRhi that
// should not be used by the widget anymore.
//
// When autoRenderTarget is true, which is the default, a depth-stencil QRhiRenderBuffer and a
// QRhiTextureRenderTarget associated with colorTexture() (or msaaColorBuffer()) and the depth-stencil
// buffer are created and managed automatically. Reimplementations of initialize() and render() can query
// those objects via depthStencilBuffer() and renderTarget(). When autoRenderTarget is set to false, these
// objects are no longer created and managed automatically. Rather, it will be up the the initialize()
// implementation to create buffers and set up the render target as it sees fit. When manually managing
// additional color or depth-stencil attachments for the render target, their size and sample count must
// always follow the size and sample count of colorTexture() / msaaColorBuffer(), otherwise rendering or 3D
// API validation errors may occur.
//
// The subclass-created graphics resources are expected to be released in the destructor implementation of
// the subclass.
//
// cb is the QRhiCommandBuffer for the current frame of the widget. The function is called with a frame
// being recorded, but without an active render pass. The command buffer is provided primarily to allow
// enqueuing resource updates without deferring to render().
void RhiWidget::initialize(QRhiCommandBuffer *cb)
{
    if (rhi_ != rhi()) {
        pipeline_.reset();
        rhi_ = rhi();
    }

    if (!pipeline_) {
        vbuf_.reset(rhi_->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(vertices)));
        vbuf_->create();

        srb_.reset(rhi_->newShaderResourceBindings());
        srb_->create();

        pipeline_.reset(rhi_->newGraphicsPipeline());
        pipeline_->setShaderStages({
            { QRhiShaderStage::Vertex, LoadShader(":/vertex.vert.qsb") },
            { QRhiShaderStage::Fragment, LoadShader(":/fragment.frag.qsb") },
        });

        QRhiVertexInputLayout layout{};
        layout.setBindings({ 5 * sizeof(float) });
        layout.setAttributes({
            { 0, 0, QRhiVertexInputAttribute::Float2, 0 },
            { 0, 1, QRhiVertexInputAttribute::Float3, 2 * sizeof(float) },
        });
        pipeline_->setVertexInputLayout(layout);
        pipeline_->setShaderResourceBindings(srb_.get());
        pipeline_->setRenderPassDescriptor(renderTarget()->renderPassDescriptor());
        pipeline_->create();

        auto rub = rhi_->nextResourceUpdateBatch();
        rub->uploadStaticBuffer(vbuf_.get(), vertices);
        cb->resourceUpdate(rub);
    }
}

// Called when the widget contents (i.e. the contents of the texture) need updating.
//
// There is always at least one call to initialize() before this function is called.
//
// To request updates, call QWidget::update(). Calling update() from within render() will lead to updating
// continuously, throttled by vsync.
//
// cb is the QRhiCommandBuffer for the current frame of the widget. The function is called with a frame
// being recorded, but without an active render pass.
void RhiWidget::render(QRhiCommandBuffer *cb)
{
    const auto rub  = rhi_->nextResourceUpdateBatch();
    const auto rtsz = renderTarget()->pixelSize();

    cb->beginPass(renderTarget(),  Qt::black, { 1.0f, 0 }, rub);

    cb->setGraphicsPipeline(pipeline_.get());
    cb->setViewport({ 0, 0, static_cast<float>(rtsz.width()), static_cast<float>(rtsz.height()) });
    cb->setShaderResources();

    const QRhiCommandBuffer::VertexInput input{ vbuf_.get(), 0 };
    cb->setVertexInput(0, 1, &input);
    cb->draw(3);

    cb->endPass();
}
