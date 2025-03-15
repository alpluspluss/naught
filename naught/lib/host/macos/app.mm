/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <stdexcept>
#include <AppKit/AppKit.h>
#include <naught/host/app.hpp>

namespace nght
{
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

    void App::run()
    {
        running = true;
        [NSApp finishLaunching];
        [NSApp activateIgnoringOtherApps:YES];
        [NSApp run];
    }

    void App::stop()
    {
        running = false;
        [NSApp terminate:nil];
    }
}