#include "z0/input_event.h"

namespace z0 {

    InputEventKey::InputEventKey(Key _key, bool _pressed, int _repeat, int _modifiers):
            InputEvent{INPUT_EVENT_KEY},
            keycode{_key},
            repeat{_repeat},
            pressed{_pressed},
            modifiers{_modifiers} {}

    InputEventMouseButton::InputEventMouseButton(MouseButton _button, bool _pressed, int _modifiers, float posX, float posY):
            InputEvent{INPUT_EVENT_MOUSE_BUTTON},
            button{_button},
            pressed{_pressed},
            modifiers{_modifiers},
            x{posX},
            y{posY} {}

    InputEventMouseMotion::InputEventMouseMotion(float posX, float posY, float rX, float rY):
            InputEvent{INPUT_EVENT_MOUSE_MOTION},
            x{posX},
            y{posY},
            relativeX{rX},
            relativeY{rY} {}

}