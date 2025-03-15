/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#pragma once

#include <naught/types.hpp>

namespace nght
{
    enum class KeyAction
    {
        UNKNOWN = -1,
        RELEASE = 0,
        PRESS = 1,
        REPEAT = 2
    };

    enum class KeyModifier
    {
        SHIFT = 0x1,
        CTRL = 0x2,

        /* ALT maps to alt key in PC keyboards; to option keys on macOS */
        ALT = 0x4,
        OPT = ALT, /* alias for ALT */

        /* SUPER maps to alt key in PC keyboards; to command keys on macOS */
        SUPER = 0x8,
        CMD = SUPER, /* alias for SUPER; for macOS use */
        ACTION
    };

    enum class KeyCode
    {
        UNKNOWN = -1,

        /* printable; ASCII keys */
        SPACE = 32,
        APOSTROPHE = 39,
        COMMA = 44,
        MINUS = 45,
        PERIOD = 46,
        SLASH = 47,
        ZERO = 48,
        ONE = 49,
        TWO,
        THREE,
        FOUR,
        FIVE,
        SIX,
        SEVEN,
        EIGHT,
        NINE,
        SEMICOLON,
        EQUAL,
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        LEFT_BRACKET, /* [ */
        BACKSLASH,
        RIGHT_BRACKET, /* ] */
        GRAVE_ACCENT, /* ` */
        WORLD_1, /* non-US #1 */
        WORLD_2, /* non-US #2 */

        /* function keys */
        ESCAPE,
        ENTER,
        TAB,
        BACKSPACE,
        INSERT,
        DELETE,
        RIGHT,
        LEFT,
        UP,
        DOWN,
        PAGE_UP,
        PAGE_DOWN,
        HOME,
        END,
        CAPSLOCK,
        SCROLL_LOCK,
        NUM_LOCK,
        PRT_SCREEN,
        PAUSE,
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        F13,
        F14,
        F15,
        F16,
        F17,
        F18,
        F19,
        F20,
        F21,
        F22,
        F23,
        F24,
        F25,

        /* num pad */
        NP0,
        NP1,
        NP2,
        NP3,
        NP4,
        NP5,
        NP6,
        NP7,
        NP8,
        NP9,
        NP_DEC,
        NP_DIV,
        NP_MUL,
        NP_SUB,
        NP_ADD,
        NP_ENTER,
        NP_EQUAL,

        LEFT_SHIFT,
        LEFT_CTRL,
        LEFT_ALT,
        LEFT_SUPER,
        RIGHT_SHIFT,
        RIGHT_CONTROL,
        RIGHT_ALT,
        RIGHT_SUPER,
        MENU,
        LAST = MENU
    };

    struct KeyInfo
    {
        KeyCode key;
        KeyAction action;
        int modifier;
    };
}