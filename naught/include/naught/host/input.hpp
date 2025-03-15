#pragma once

#include <functional>
#include <memory>
#include <naught/types.hpp>
#include <naught/host/key.hpp>

namespace nght
{
    class Input
    {
    public:
        Input();

        ~Input();

        /* query method */
        bool is_key_pressed(KeyCode k) const;

        bool is_key_released(KeyCode k) const;

        bool is_mbtn_pressed(int btn) const; /* 0: left, 1: right, 2: middle */

        Vec2 mouse_pos() const;

        Vec2 mouse_delta() const;

        Vec2 scroll_delta() const;

        void reset_deltas();

        std::function<bool(const KeyInfo&)> on_key_event;
        std::function<void(const Vec2&, const Vec2&)> on_mouse_move;
        std::function<void(int, bool, int)> on_mouse_button;
        std::function<void(float, float)> on_scroll;

        bool proc_key_event(KeyCode key, KeyAction action, int modifiers);
        void proc_mouse_move(float x, float y);
        void proc_mouse_button(int button, bool pressed, int modifiers);
        void proc_scroll(float x_offset, float y_offset);
        
    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl;
    };
}