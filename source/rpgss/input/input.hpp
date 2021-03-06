/*
    The MIT License (MIT)

    Copyright (c) 2014 Anatoli Steinmark

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#ifndef RPGSS_INPUT_INPUT_HPP_INCLUDED
#define RPGSS_INPUT_INPUT_HPP_INCLUDED

#include "../core/Vec2.hpp"


namespace rpgss {
    namespace input {

        enum Key {
            KEY_BACKSPACE = 0,
            KEY_TAB,
            KEY_CLEAR,
            KEY_ENTER,
            KEY_SHIFT,
            KEY_CONTROL,
            KEY_ALT,
            KEY_CAPSLOCK,
            KEY_ESCAPE,
            KEY_SPACE,
            KEY_PAGEUP,
            KEY_PAGEDOWN,
            KEY_END,
            KEY_HOME,
            KEY_LEFT,
            KEY_UP,
            KEY_RIGHT,
            KEY_DOWN,
            KEY_PRINTSCREEN,
            KEY_INSERT,
            KEY_DELETE,
            KEY_0,
            KEY_1,
            KEY_2,
            KEY_3,
            KEY_4,
            KEY_5,
            KEY_6,
            KEY_7,
            KEY_8,
            KEY_9,
            KEY_A,
            KEY_B,
            KEY_C,
            KEY_D,
            KEY_E,
            KEY_F,
            KEY_G,
            KEY_H,
            KEY_I,
            KEY_J,
            KEY_K,
            KEY_L,
            KEY_M,
            KEY_N,
            KEY_O,
            KEY_P,
            KEY_Q,
            KEY_R,
            KEY_S,
            KEY_T,
            KEY_U,
            KEY_V,
            KEY_W,
            KEY_X,
            KEY_Y,
            KEY_Z,
            KEY_NUMPAD0,
            KEY_NUMPAD1,
            KEY_NUMPAD2,
            KEY_NUMPAD3,
            KEY_NUMPAD4,
            KEY_NUMPAD5,
            KEY_NUMPAD6,
            KEY_NUMPAD7,
            KEY_NUMPAD8,
            KEY_NUMPAD9,
            KEY_MULTIPLY,
            KEY_ADD,
            KEY_SUBTRACT,
            KEY_DECIMAL,
            KEY_DIVIDE,
            KEY_F1,
            KEY_F2,
            KEY_F3,
            KEY_F4,
            KEY_F5,
            KEY_F6,
            KEY_F7,
            KEY_F8,
            KEY_F9,
            KEY_F10,
            KEY_F11,
            KEY_F12,
            KEY_NUMLOCK,
            KEY_SCROLLLOCK,
            KEY_LSHIFT,
            KEY_RSHIFT,
            KEY_LCONTROL,
            KEY_RCONTROL,
            KEY_LALT,
            KEY_RALT,
            NUMKEYS,
        };

        enum MouseButton {
            MOUSEBUTTON_LEFT = 0,
            MOUSEBUTTON_RIGHT,
            MOUSEBUTTON_MIDDLE,
            MOUSEBUTTON_X1,
            MOUSEBUTTON_X2,
            NUMMOUSEBUTTONS,
        };

        int   GetVirtualKeyCode(int key);
        bool  IsKeyPressed(int key);
        bool  IsAnyKeyPressed();
        core::Vec2i GetMousePosition();
        bool  IsMouseButtonPressed(int mbutton);
        bool  IsAnyMouseButtonPressed();

    } // namespace input
} // namespace rpgss


#endif // RPGSS_INPUT_INPUT_HPP_INCLUDED
