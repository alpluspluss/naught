/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include "naught/host/key.hpp"
#include <memory>
#include <map>
#include <objc/NSObjCRuntime.h>
#include <optional>
#include <AppKit/AppKit.h>
#include <naught/host/input.hpp>

namespace nght
{
    struct Input::Impl
    {
        std::map<KeyCode, KeyAction> key_states;
        Vec2 mouse_pos{0.0f, 0.0f};
        Vec2 mouse_delta{0.0f, 0.0f};
        Vec2 scroll_delta{0.0f, 0.0f};
        bool mouse_buttons[3] = { false, false, false }; /* left, right, middle */
    };

    Input::Input() : pimpl(std::make_unique<Impl>()) {}

    Input::~Input() = default;

    bool Input::is_key_pressed(KeyCode k) const
    {
        auto it = pimpl->key_states.find(k);
        return (it != pimpl->key_states.end() && 
               (it->second == KeyAction::PRESS || it->second == KeyAction::REPEAT));
    }

    bool Input::is_key_released(KeyCode k) const
    {
        auto it = pimpl->key_states.find(k);
        return (it != pimpl->key_states.end() && it->second == KeyAction::RELEASE);
    }

    bool Input::is_mbtn_pressed(int btn) const
    {
        if (btn >= 0 && btn < 3)
            return pimpl->mouse_buttons[btn];
        return false;
    }

    Vec2 Input::mouse_pos() const
    {
        return pimpl->mouse_pos;
    }
    
    Vec2 Input::mouse_delta() const
    {
        return pimpl->mouse_delta;
    }
    
    Vec2 Input::scroll_delta() const
    {
        return pimpl->scroll_delta;
    }
    
    void Input::reset_deltas()
    {
        pimpl->mouse_delta = {0.0f, 0.0f};
        pimpl->scroll_delta = {0.0f, 0.0f};
    }

    bool Input::proc_key_event(KeyCode key, KeyAction action, int modifiers)
    {
        pimpl->key_states[key] = action;
        if (on_key_event)
            return on_key_event({key, action, modifiers});
        
        return false;
    }
    
    void Input::proc_mouse_move(float x, float y)
    {
        Vec2 new_pos{x, y};
        pimpl->mouse_delta = {
            new_pos.first - pimpl->mouse_pos.first,
            new_pos.second - pimpl->mouse_pos.second
        };
        pimpl->mouse_pos = new_pos;
        
        if (on_mouse_move)
            on_mouse_move(new_pos, pimpl->mouse_delta);
    }
    
    void Input::proc_mouse_button(int button, bool pressed, int modifiers)
    {
        if (button >= 0 && button < 3)
            pimpl->mouse_buttons[button] = pressed;
        
        if (on_mouse_button)
            on_mouse_button(button, pressed, modifiers);
    }
    
    void Input::proc_scroll(float x_offset, float y_offset)
    {
        pimpl->scroll_delta = {x_offset, y_offset};
        
        if (on_scroll)
            on_scroll(x_offset, y_offset);
    }
}

namespace nght
{
    KeyCode get_keycode(uint8_t keycode)
    {
        switch (keycode)
        {
            case 0x00: return KeyCode::A;
            case 0x0B: return KeyCode::B;
            case 0x08: return KeyCode::C;
            case 0x02: return KeyCode::D;
            case 0x0E: return KeyCode::E;
            case 0x03: return KeyCode::F;
            case 0x05: return KeyCode::G;
            case 0x04: return KeyCode::H;
            case 0x22: return KeyCode::I;
            case 0x26: return KeyCode::J;
            case 0x28: return KeyCode::K;
            case 0x25: return KeyCode::L;
            case 0x2E: return KeyCode::M;
            case 0x2D: return KeyCode::N;
            case 0x1F: return KeyCode::O;
            case 0x23: return KeyCode::P;
            case 0x0C: return KeyCode::Q;
            case 0x0F: return KeyCode::R;
            case 0x01: return KeyCode::S;
            case 0x11: return KeyCode::T;
            case 0x20: return KeyCode::U;
            case 0x09: return KeyCode::V;
            case 0x0D: return KeyCode::W;
            case 0x07: return KeyCode::X;
            case 0x10: return KeyCode::Y;
            case 0x06: return KeyCode::Z;
            
            case 0x1D: return KeyCode::ZERO;
            case 0x12: return KeyCode::ONE;
            case 0x13: return KeyCode::TWO;
            case 0x14: return KeyCode::THREE;
            case 0x15: return KeyCode::FOUR;
            case 0x17: return KeyCode::FIVE;
            case 0x16: return KeyCode::SIX;
            case 0x1A: return KeyCode::SEVEN;
            case 0x1C: return KeyCode::EIGHT;
            case 0x19: return KeyCode::NINE;
            
            case 0x35: return KeyCode::ESCAPE;
            case 0x24: return KeyCode::ENTER;
            case 0x30: return KeyCode::TAB;
            case 0x33: return KeyCode::BACKSPACE;
            case 0x72: return KeyCode::INSERT;
            case 0x75: return KeyCode::DELETE;
            case 0x7C: return KeyCode::RIGHT;
            case 0x7B: return KeyCode::LEFT;
            case 0x7E: return KeyCode::UP;
            case 0x7D: return KeyCode::DOWN;
            case 0x74: return KeyCode::PAGE_UP;
            case 0x79: return KeyCode::PAGE_DOWN;
            case 0x73: return KeyCode::HOME;
            case 0x77: return KeyCode::END;
            case 0x39: return KeyCode::CAPSLOCK;
            
            case 0x7A: return KeyCode::F1;
            case 0x78: return KeyCode::F2;
            case 0x63: return KeyCode::F3;
            case 0x76: return KeyCode::F4;
            case 0x60: return KeyCode::F5;
            case 0x61: return KeyCode::F6;
            case 0x62: return KeyCode::F7;
            case 0x64: return KeyCode::F8;
            case 0x65: return KeyCode::F9;
            case 0x6D: return KeyCode::F10;
            case 0x67: return KeyCode::F11;
            case 0x6F: return KeyCode::F12;
            
            case 0x31: return KeyCode::SPACE;
            case 0x1B: return KeyCode::MINUS;
            case 0x18: return KeyCode::EQUAL;
            case 0x21: return KeyCode::LEFT_BRACKET;
            case 0x1E: return KeyCode::RIGHT_BRACKET;
            case 0x27: return KeyCode::SEMICOLON;
            case 0x29: return KeyCode::APOSTROPHE;
            case 0x2A: return KeyCode::BACKSLASH;
            case 0x2B: return KeyCode::COMMA;
            case 0x2F: return KeyCode::PERIOD;
            case 0x2C: return KeyCode::SLASH;
            case 0x32: return KeyCode::GRAVE_ACCENT;
            
            /* mod */
            case 0x38: return KeyCode::LEFT_ALT;      /* left option */
            case 0x3B: return KeyCode::LEFT_CTRL;     /* left vontrol */
            case 0x37: return KeyCode::LEFT_SUPER;    /* left command */
            case 0x3A: return KeyCode::RIGHT_ALT;      /* right option */
            case 0x3E: return KeyCode::RIGHT_CONTROL; /* right control */
            case 0x36: return KeyCode::RIGHT_SUPER;   /* right command */
            case 0x3C: return KeyCode::RIGHT_SHIFT;   /* right shift */
            case 0x3D: return KeyCode::RIGHT_SHIFT;   /* right option */
            
            default: return KeyCode::UNKNOWN;
        }
    }

    int get_modfl(NSUInteger flags)
    {
        int mods = 0;
        
        if (flags & NSEventModifierFlagShift)
            mods |= static_cast<int>(KeyModifier::SHIFT);
        
        if (flags & NSEventModifierFlagControl)
            mods |= static_cast<int>(KeyModifier::CTRL);
        
        if (flags & NSEventModifierFlagOption)
            mods |= static_cast<int>(KeyModifier::ALT);
        
        if (flags & NSEventModifierFlagCommand)
            mods |= static_cast<int>(KeyModifier::SUPER);
        
        return mods;
    }
}