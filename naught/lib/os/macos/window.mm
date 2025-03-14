/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <memory>
#include <stdexcept>
#include <objc/NSObject.h>
#include <objc/objc.h>
#include <naught/types.hpp>
#include <naught/window.hpp>
#include <AppKit/AppKit.h>
#include <Foundation/Foundation.h>

@interface NaughtWindowDelegate : NSObject<NSWindowDelegate>
{
    nght::NaughtWindow* nwindow; /* naught window */
    
}
@end

@implementation NaughtWindowDelegate

- (instancetype)initWithWindow:(nght::NaughtWindow*)window
{
    if (self = [super init])
        nwindow = window;
    return self;
}

- (BOOL)windowShouldClose:(NSWindow *)sender
{
    if (nwindow->on_close)
        nwindow->on_close();
    return YES;
}

- (void)windowDidResize:(NSNotification *)notification
{
    if (nwindow->on_resize)
        nwindow->on_resize();
}
@end

namespace nght
{
    struct NaughtWindow::Impl
    {
        NSWindow* window;
        NaughtWindowDelegate* delegate = {};

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
                id app_name = [ [NSProcessInfo processInfo] processName];
                id quit_tt = [@"Quit" stringByAppendingString:app_name];
                id quit_mn_item = [ [NSMenuItem alloc] initWithTitle:quit_tt 
                                    action:@selector(terminate:) keyEquivalent:@"q"];

                [app_menu addItem:quit_mn_item];
                [app_menu_item setSubmenu:app_menu];
            }
        }

        ~Impl()
        {
            if (window)
            {
                [window setDelegate:nil];
                [window close];
                [window release];
            }

            if (delegate)
                [delegate release];
        }

        void set_delegate(NaughtWindow* window_ptr) 
        {
            delegate = [[NaughtWindowDelegate alloc] initWithWindow:window_ptr];
            [window setDelegate:delegate];
        }
    };

    NaughtWindow::NaughtWindow(const std::string& name, Style style, const Rect& bounds)
    {
        pimpl = std::make_unique<Impl>(name, style, bounds);
        pimpl->set_delegate(this);

        [pimpl->window makeKeyAndOrderFront:nil];
        [NSApp activateIgnoringOtherApps:YES];
    }

    NaughtWindow::~NaughtWindow() = default; /* in objc++, AppDelegate already handles RAII for us */

    void* NaughtWindow::native_handle() const
    {
        return pimpl->window;
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
        NSRect screen = [ [NSScreen mainScreen] frame];

        frame.origin.x = p.first;
        frame.origin.y = screen.size.height - p.second - frame.size.height;

        [pimpl->window setFrame:frame display:YES animate:NO];
    }
}