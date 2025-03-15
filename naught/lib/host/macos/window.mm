/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <memory>
#include <stdexcept>
#include <naught/types.hpp>
#include <objc/NSObject.h>
#include <objc/objc.h>
#include <AppKit/AppKit.h>
#include <Foundation/Foundation.h>
#include <naught/host/window.hpp>
#include <naught/host/view.hpp>
#include <naught/host/input.hpp>

extern "C" 
{
    void nght_window_handle_close(void* window_ptr);
    void nght_window_handle_resize(void* window_ptr);
    void nght_window_handle_key(void* window_ptr, unsigned short key_code, int action, NSUInteger flags);
    void nght_window_handle_mouse_move(void* window_ptr, NSPoint point);
    void nght_window_handle_mouse_button(void* window_ptr, int button, BOOL isDown, NSUInteger flags);
    void nght_window_handle_scroll(void* window_ptr, float dx, float dy);
}

namespace nght
{
    /* translate functions */
    KeyCode get_keycode(uint8_t keycode); /* get keycode */
    int get_modfl(NSUInteger flags); /* get mod flags */
}

@interface NaughtWindowDelegate : NSObject<NSWindowDelegate>
{
    void* nwindow_ptr; /* naught window pointer */
}
@end

@implementation NaughtWindowDelegate

- (instancetype)initWithWindow:(void*)window
{
    if (self = [super init])
        nwindow_ptr = window;
    return self;
}

- (BOOL)windowShouldClose:(NSWindow *)sender
{
    nght_window_handle_close(nwindow_ptr);
    return YES;
}

- (void)windowDidResize:(NSNotification *)notification
{
    nght_window_handle_resize(nwindow_ptr);
}
@end

@interface NaughtContentView : NSView
{
    void* nwindow_ptr; /* naught window pointer */
}
@end

@implementation NaughtContentView

- (instancetype)initWithFrame:(NSRect)frameRect window:(void*)window
{
    if (self = [super initWithFrame:frameRect]) 
    {
        nwindow_ptr = window;

        self.wantsLayer = YES;
        self.allowedTouchTypes = NSTouchTypeMaskDirect | NSTouchTypeMaskIndirect;
        
        NSTrackingAreaOptions options = (NSTrackingActiveAlways | NSTrackingInVisibleRect |
                                         NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved);
        NSTrackingArea *trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds]
                                                                    options:options
                                                                      owner:self
                                                                   userInfo:nil];
        [self addTrackingArea:trackingArea];
    }
    return self;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (BOOL)canBecomeKeyView
{
    return YES;
}

/* keyboard events */
- (void)keyDown:(NSEvent *)event
{
    nght_window_handle_key(nwindow_ptr, [event keyCode], 1, [event modifierFlags]);
}

- (void)keyUp:(NSEvent *)event
{
    nght_window_handle_key(nwindow_ptr, [event keyCode], 0, [event modifierFlags]);
}

- (void)flagsChanged:(NSEvent *)event
{
    static NSUInteger lastFlags = 0;
    NSUInteger flags = [event modifierFlags];
    NSUInteger changed = lastFlags ^ flags;
    
    BOOL keyDown = (flags & changed) != 0;
    
    if (changed & NSEventModifierFlagShift ||
        changed & NSEventModifierFlagControl ||
        changed & NSEventModifierFlagOption ||
        changed & NSEventModifierFlagCommand) 
    {
        nght_window_handle_key(nwindow_ptr, [event keyCode], 
                                        keyDown ? 1 : 0, flags);
    }
    
    lastFlags = flags;
}

/* mouse event */
- (void)mouseMoved:(NSEvent *)event
{
    NSPoint location = [self convertPoint:[event locationInWindow] fromView:nil];
    nght_window_handle_mouse_move(nwindow_ptr, location);
}

- (void)mouseDragged:(NSEvent *)event
{
    NSPoint location = [self convertPoint:[event locationInWindow] fromView:nil];
    nght_window_handle_mouse_move(nwindow_ptr, location);
}

- (void)rightMouseDragged:(NSEvent *)event
{
    NSPoint location = [self convertPoint:[event locationInWindow] fromView:nil];
    nght_window_handle_mouse_move(nwindow_ptr, location);
}

- (void)otherMouseDragged:(NSEvent *)event
{
    NSPoint location = [self convertPoint:[event locationInWindow] fromView:nil];
    nght_window_handle_mouse_move(nwindow_ptr, location);
}

- (void)mouseDown:(NSEvent *)event
{
    nght_window_handle_mouse_button(nwindow_ptr, 0, YES, [event modifierFlags]);
}

- (void)mouseUp:(NSEvent *)event
{
    nght_window_handle_mouse_button(nwindow_ptr, 0, NO, [event modifierFlags]);
}

- (void)rightMouseDown:(NSEvent *)event
{
    nght_window_handle_mouse_button(nwindow_ptr, 1, YES, [event modifierFlags]);
}

- (void)rightMouseUp:(NSEvent *)event
{
    nght_window_handle_mouse_button(nwindow_ptr, 1, NO, [event modifierFlags]);
}

- (void)otherMouseDown:(NSEvent *)event
{
    if ([event buttonNumber] == 2)
        nght_window_handle_mouse_button(nwindow_ptr, 2, YES, [event modifierFlags]);
}

- (void)otherMouseUp:(NSEvent *)event
{
    if ([event buttonNumber] == 2)
        nght_window_handle_mouse_button(nwindow_ptr, 2, NO, [event modifierFlags]);
}

- (void)scrollWheel:(NSEvent *)event
{
    float dx = [event scrollingDeltaX];
    float dy = [event scrollingDeltaY];
    
    if ([event hasPreciseScrollingDeltas])
    {
        dx /= 10.0;
        dy /= 10.0;
    }
    
    nght_window_handle_scroll(nwindow_ptr, dx, dy);
}

@end

void nght_window_handle_close(void* window_ptr)
{
    auto* window = static_cast<nght::NaughtWindow*>(window_ptr);
    if (window->on_close)
        window->on_close();
}

void nght_window_handle_resize(void* window_ptr)
{
    auto* window = static_cast<nght::NaughtWindow*>(window_ptr);
    if (window->on_resize)
        window->on_resize();
        
    /* also notify view if it exists */
    if (window->view() && window->view()->on_resize)
        window->view()->on_resize(window->size());
}

void nght_window_handle_key(void* window_ptr, unsigned short key_code, int action, NSUInteger flags)
{
    auto* window = static_cast<nght::NaughtWindow*>(window_ptr);
    if (!window || !window->input())
        return;
    
    nght::KeyCode key = nght::get_keycode(key_code);
    nght::KeyAction act = (action == 0) ? nght::KeyAction::RELEASE : nght::KeyAction::PRESS;
    int mods = nght::get_modfl(flags);
    
    window->input()->proc_key_event(key, act, mods);
}

void nght_window_handle_mouse_move(void* window_ptr, NSPoint point)
{
    auto* window = static_cast<nght::NaughtWindow*>(window_ptr);
    if (!window || !window->input())
        return;
    
    window->input()->proc_mouse_move(point.x, point.y);
}

void nght_window_handle_mouse_button(void* window_ptr, int button, BOOL isDown, NSUInteger flags)
{
    auto* window = static_cast<nght::NaughtWindow*>(window_ptr);
    if (!window || !window->input())
        return;
    
    int mods = nght::get_modfl(flags);
    window->input()->proc_mouse_button(button, isDown == YES, mods);
}

void nght_window_handle_scroll(void* window_ptr, float dx, float dy)
{
    auto* window = static_cast<nght::NaughtWindow*>(window_ptr);
    if (!window || !window->input())
        return;
    
    window->input()->proc_scroll(dx, dy);
}

namespace nght
{
    struct NaughtWindow::Impl
    {
        NSWindow* window;
        NaughtWindowDelegate* delegate = {};
        NaughtContentView* content_view = nullptr;
        std::unique_ptr<View> view_ptr;
        std::unique_ptr<Input> input_ptr;

        Impl(const std::string& name, Style style, const Rect& bounds)
        {
            NSWindowStyleMask style_mask = 0;
            if (static_cast<int>(style) & static_cast<int>(Style::WITH_TITLE))
                style_mask |= NSWindowStyleMaskTitled;
            if (static_cast<int>(style) & static_cast<int>(Style::CLOSABLE))
                style_mask |= NSWindowStyleMaskClosable;
            if (static_cast<int>(style) & static_cast<int>(Style::MINIATURIZABLE))
                style_mask |= NSWindowStyleMaskMiniaturizable;
            if (static_cast<int>(style) & static_cast<int>(Style::RESIZABLE))
                style_mask |= NSWindowStyleMaskResizable;

            NSRect content = NSMakeRect(100, 100, bounds.first, bounds.second);
            window = [[NSWindow alloc]
                initWithContentRect:content
                styleMask:style_mask
                backing:NSBackingStoreBuffered
                defer:NO];
            
            if (!window)
                throw std::runtime_error("failed to create NaughtWindow");
            
            [window setTitle:[NSString stringWithUTF8String:name.c_str()]];
            [window setReleasedWhenClosed:NO];
            [window center];

            /* init app */
            [NSApplication sharedApplication];
            [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

            if ([NSApp mainMenu] == nil)
            {
                id menu_bar = [NSMenu new];
                id app_menu_item = [NSMenuItem new];

                [menu_bar addItem:app_menu_item];
                [NSApp setMainMenu:menu_bar];

                id app_menu = [NSMenu new];
                id app_name = [[NSProcessInfo processInfo] processName];
                id quit_tt = [@"Quit" stringByAppendingString:app_name];
                id quit_mn_item = [[NSMenuItem alloc] initWithTitle:quit_tt 
                                    action:@selector(terminate:) keyEquivalent:@"q"];

                [app_menu addItem:quit_mn_item];
                [app_menu_item setSubmenu:app_menu];
            }
        }

        ~Impl()
        {
            view_ptr.reset();
            input_ptr.reset();
            
            if (window)
            {
                [window setDelegate:nil];
                [window close];
                [window release];
            }

            if (delegate)
                [delegate release];
        }

        void setup_window(NaughtWindow* window_ptr) 
        {
            NSRect frame = [[window contentView] frame];
            content_view = [[NaughtContentView alloc] initWithFrame:frame window:window_ptr];
            [window setContentView:content_view];
            
            delegate = [[NaughtWindowDelegate alloc] initWithWindow:window_ptr];
            [window setDelegate:delegate];
            
            [window makeKeyAndOrderFront:nil];
        }
    };

    NaughtWindow::NaughtWindow(const std::string& name, Style style, const Rect& bounds)
    {
        pimpl = std::make_unique<Impl>(name, style, bounds);
        pimpl->setup_window(this);
        [NSApp activateIgnoringOtherApps:YES];
    }

    NaughtWindow::~NaughtWindow() = default;

    void* NaughtWindow::native_handle() const
    {
        return pimpl->window;
    }
    
    View* NaughtWindow::view() const
    {
        return pimpl->view_ptr.get();
    }
    
    View* NaughtWindow::create_view()
    {
        if (!pimpl->view_ptr)
            pimpl->view_ptr = std::make_unique<View>(native_handle());
        
        return pimpl->view_ptr.get();
    }
    
    Input* NaughtWindow::input() const
    {
        return pimpl->input_ptr.get();
    }
    
    Input* NaughtWindow::create_input()
    {
        if (!pimpl->input_ptr)
            pimpl->input_ptr = std::make_unique<Input>();
        
        return pimpl->input_ptr.get();
    }

    Vec2 NaughtWindow::size() const
    {
        NSRect frame = [pimpl->window.contentView frame];
        return Vec2{static_cast<float>(frame.size.width), static_cast<float>(frame.size.height)};
    }

    void NaughtWindow::size(const Vec2& p)
    {
        NSRect frame = [pimpl->window frame];
        NSRect content = [pimpl->window contentRectForFrameRect:frame];
        
        CGFloat delta_width = frame.size.width - content.size.width;
        CGFloat delta_height = frame.size.height - content.size.height;
        
        /* set new size including the frame */
        frame.size.width = p.first + delta_width;
        frame.size.height = p.second + delta_height;
        
        [pimpl->window setFrame:frame display:YES animate:NO];
    }

    Vec2 NaughtWindow::position() const
    {
        NSRect frame = [pimpl->window frame];
        return Vec2{static_cast<float>(frame.origin.x),
            static_cast<float>([[NSScreen mainScreen] frame].size.height - frame.origin.y - frame.size.height)};
    }

    void NaughtWindow::position(const Vec2& p)
    {
        NSRect frame = [pimpl->window frame];
        NSRect screen = [[NSScreen mainScreen] frame];

        frame.origin.x = p.first;
        frame.origin.y = screen.size.height - p.second - frame.size.height;

        [pimpl->window setFrame:frame display:YES animate:NO];
    }
}