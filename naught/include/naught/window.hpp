/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#pragma once

#include <string>
#include <functional>
#include <memory>
#include <naught/types.hpp>

namespace nght
{
    class View;

    class NaughtWindow
    {
    public:
        enum class Style
        {
            BARE = 0x0,
            WITH_TITLE = 0x1,
            CLOSABLE = 0x2,
            MINIATURIZABLE = 0x4,
            RESIZABLE = 8,
            STANDARD = WITH_TITLE | CLOSABLE | MINIATURIZABLE | RESIZABLE
        };

        NaughtWindow(const std::string& name, Style style = Style::STANDARD, const Rect& bounds = Rect(1920, 1080));
        
        NaughtWindow(Style style = Style::STANDARD, const Rect& bounds = Rect(1920, 1080)) : NaughtWindow("", style, bounds) {}

        ~NaughtWindow();

        /* non-copyable */
        NaughtWindow(const NaughtWindow&) = delete;

        NaughtWindow& operator=(const NaughtWindow&) = delete;

        /* movable */
        NaughtWindow(NaughtWindow&&) noexcept = default;

        NaughtWindow& operator=(NaughtWindow&&) noexcept = default;

        /* render view layer */
        View* view() const;

        View* create_view();

        /* transform methods */
        [[nodiscard]] Vec2 size() const;

        void size(const Vec2& p);

        [[nodiscard]] Vec2 position() const;

        void position(const Vec2& p);

        void* native_handle() const;

        std::function<void()> on_close;

        std::function<void()> on_resize;

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl;

    };

    /* bitwise for style enum */
    inline NaughtWindow::Style operator|(NaughtWindow::Style a, NaughtWindow::Style b)
    {
        return static_cast<NaughtWindow::Style>(static_cast<int>(a) | static_cast<int>(b));
    }
}