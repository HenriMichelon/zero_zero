#include "z0/z0.h"

namespace z0 {

    InputEventKey::InputEventKey(Key _key, bool _pressed, int _repeat, int _modifiers):
            InputEvent{INPUT_EVENT_KEY},
            keycode{_key},
            repeat{_repeat},
            pressed{_pressed},
            modifiers{_modifiers} {}

    InputEventGamepadButton::InputEventGamepadButton(GamepadButton _button, bool _pressed):
            InputEvent{INPUT_EVENT_GAMEPAD_BUTTON},
            button{_button},
            pressed{_pressed} {}

    InputEventMouseButton::InputEventMouseButton(MouseButton _button, bool _pressed, int modifiers, uint32_t buttonsState, float posX, float posY):
            InputEventMouse{INPUT_EVENT_MOUSE_BUTTON, buttonsState, modifiers, posX, posY},
            button{_button},
            pressed{_pressed} {}

    InputEventMouseMotion::InputEventMouseMotion(uint32_t buttonsState, int modifiers, float posX, float posY, float rX, float rY):
            InputEventMouse{INPUT_EVENT_MOUSE_MOTION, buttonsState, modifiers, posX, posY},
            relativeX{rX},
            relativeY{rY} {}

    InputEventMouse::InputEventMouse(InputEventType type, uint32_t _buttonsState, int _modifiers, float posX, float posY):
            InputEvent{type},
            x{posX},
            y{posY},
            buttonsState{_buttonsState},
            modifiers{_modifiers} {}

}