/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <stdexcept>
#include <AppKit/AppKit.h>
#include <naught/host/app.hpp>
#include <naught/host/view.hpp>

namespace nght
{
    App& App::get() 
    {
        static App instance;
        return instance;
    }

    App::App() : running(false)
    {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

        NSString* ns_app_name = [[NSProcessInfo processInfo] processName];
        app_name = [ns_app_name UTF8String];

        if ([NSApp mainMenu] == nil)
        {
            NSMenu* menu_bar = [[NSMenu alloc] init];
            NSMenuItem* app_mn_item = [[NSMenuItem alloc] init];
            [menu_bar addItem:app_mn_item];
            [NSApp setMainMenu:menu_bar];

            NSMenu* appmn = [[NSMenu alloc] init];
            NSString* quit_tt = [@"Quit " stringByAppendingString:ns_app_name];
            NSMenuItem* quit_mn_item = [[NSMenuItem alloc] initWithTitle:quit_tt
                                                                  action:@selector(terminate:)
                                                           keyEquivalent:@"q"];
            [appmn addItem:quit_mn_item];
            [app_mn_item setSubmenu:appmn];
        }
    }

    App::~App() = default;

    const std::string& App::name() const
    {
        return app_name;
    }

    NaughtWindow* App::window(const WindowConfig& config)
    {
        auto window = std::make_unique<NaughtWindow>(
            config.title,
            config.style,
            config.bounds
        );

        if (!config.center)
            window->position(config.position);

        window->view()->clear(config.clear_color[0], config.clear_color[1], config.clear_color[2], config.clear_color[3]);

        auto* window_ptr = window.get();
        windows.push_back(std::move(window));

        return window_ptr;
    }

    NaughtWindow* App::main_window() const
    {
        return windows.empty() ? nullptr : windows[0].get();
    }

    void App::run()
    {
        running = true;
        [NSApp finishLaunching];
        [NSApp activateIgnoringOtherApps:YES];

        if (!windows.empty())
            [NSApp run];
    }

    void App::stop()
    {
        running = false;
        [NSApp terminate:nil];
    }
}