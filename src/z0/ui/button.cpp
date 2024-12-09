/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;

module z0.ui.Button;

import z0.ui.Event;

namespace z0 {
    namespace ui {

        Button::Button(): Box{BUTTON} {
            moveChildrenOnPush = true;
            redrawOnMouseEvent = true;
            allowFocus = true;
        }

        bool Button::eventMouseUp(const MouseButton B, const float X, const float Y) {
            const bool p = isPushed();
            if (p && (!getRect().contains(X, Y))) {
                setPushed(false);
                resizeChildren();
                return Box::eventMouseUp(B, X, Y);
            }
            const bool consumed = Box::eventMouseUp(B, X, Y);
            if ((!consumed) && p) {
                auto event = EventClick{};
                emit(Event::OnClick, &event);
                return event.consumed;
            }
            return consumed;
        }
    }
}
