/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include <memory>
#include <string>
#include <vector>
#include <naught/host/window.hpp>

namespace nght
{
    struct WindowConfig
    {
        std::string title = { "Naught" };
        Rect bounds = { 800.0f, 600.0f };
        NaughtWindow::Style style = NaughtWindow::Style::STANDARD;
        bool center = true;
        Vec2 position = { 100.0f, 100.0f };
        float clear_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    };

    class App
    {
    public:

        static App& get();

        App(const App&) = delete;
        App& operator=(const App&) = delete;

        [[nodiscard]] const std::string& name() const;

        NaughtWindow* window(const WindowConfig& config = {});

        [[nodiscard]] NaughtWindow* main_window() const;

        void run();

        void stop();

    private:
        App();
        ~App();

        std::string app_name;
        bool running{false};
        std::vector<std::unique_ptr<NaughtWindow>> windows;
    };
}