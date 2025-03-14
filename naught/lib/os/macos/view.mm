/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <cstddef>
#include <memory>
#include <objc/NSObject.h>
#include <objc/objc.h>
#include <stdexcept>
#include <naught/view.hpp>
#include <AppKit/AppKit.h>
#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>

@interface NaughtMetalView : NSView
@end

@implementation NaughtMetalView
- (BOOL)isOpaque
{
    return YES;
}

- (BOOL)wantsLayer 
{
    return YES;
}
@end

namespace nght
{
    struct View::Impl
    {
        void* window_handle;
        NSView* view = nil;
        CAMetalLayer* metal_layer = nil;

        id<MTLDevice> device = nil;
        id<MTLCommandQueue> cmdq = nil;

        id<CAMetalDrawable> current_drawable = nil;
        id<MTLCommandBuffer> current_cmd_buffer = nil;

        MTLClearColor clear_color = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);

        Impl(void* window) : window_handle(window)
        {
            device = MTLCreateSystemDefaultDevice();
            if (!device)
                throw std::runtime_error("metal is not supported on this device");
            
            /* setup metal layer */
            NSWindow* win = (__bridge NSWindow*)window_handle;
            view = win.contentView;
            if (!view)
                throw std::runtime_error("window has no content view");
            
            /* conf the layer */
            metal_layer = [CAMetalLayer layer];
            metal_layer.device = device;
            metal_layer.pixelFormat = MTLPixelFormatABGR4Unorm;
            metal_layer.framebufferOnly = YES;
            metal_layer.contentsScale = win.backingScaleFactor;

            view.wantsLayer = YES;
            view.layer = metal_layer;

            cmdq = [device newCommandQueue];
            if (!cmdq)
                throw std::runtime_error("failed to create metal command queue");
        }

        ~Impl() = default; /* objc++ objects are automatically released by default */
    };

    View::View(void* window_handle) : pimpl(std::make_unique<Impl>(window_handle)), on_render(nullptr), on_resize(nullptr) {}

    View::~View() = default;

    Vec2 View::size() const
    {
        NSView* view = pimpl->view;
        NSRect bounds = [view bounds];
        return Vec2{ static_cast<float>(bounds.size.width), static_cast<float>(bounds.size.height) };
    }

    void View::clear(float r, float g, float b, float a)
    {
        pimpl->clear_color = MTLClearColorMake(r, g, b, a);
    }

    bool View::begin()
    {
        pimpl->current_drawable = [pimpl->metal_layer nextDrawable];
        if (!pimpl->current_drawable)
            return false;
        
        pimpl->current_cmd_buffer = [pimpl->cmdq commandBuffer];
        return true;
    }

    void View::end()
    {
        if (!pimpl->current_cmd_buffer || !pimpl->current_drawable) {
            return;
        }
        
        MTLRenderPassDescriptor* render_pass = [MTLRenderPassDescriptor renderPassDescriptor];
        render_pass.colorAttachments[0].texture = pimpl->current_drawable.texture;
        render_pass.colorAttachments[0].clearColor = pimpl->clear_color;
        render_pass.colorAttachments[0].loadAction = MTLLoadActionClear;
        render_pass.colorAttachments[0].storeAction = MTLStoreActionStore;
        
        /* make render encoder */
        id<MTLRenderCommandEncoder> encoder = [pimpl->current_cmd_buffer renderCommandEncoderWithDescriptor:render_pass];
        [encoder endEncoding];
        
        /* present the drawable to the screen */
        [pimpl->current_cmd_buffer presentDrawable:pimpl->current_drawable];
        [pimpl->current_cmd_buffer commit];
        
        /* reset for next frame */
        pimpl->current_drawable = nil;
        pimpl->current_cmd_buffer = nil;
    }

    void* View::native_view() const
    {
        return (__bridge void*)pimpl->view;
    }
    
    void* View::native_layer() const
    {
        return (__bridge void*)pimpl->metal_layer;
    }
    
    void* View::native_device() const
    {
        return (__bridge void*)pimpl->device;
    }
    
    void* View::native_cmdq() const
    {
        return (__bridge void*)pimpl->cmdq;
    }
}