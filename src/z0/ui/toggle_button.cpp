/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

module z0.ui.ToggleButton;

import z0.ui.Event;

namespace z0 {
    namespace ui {

        ToggleButton::ToggleButton(): CheckWidget(TOGGLEBUTTON) {
            moveChildrenOnPush = true;
            redrawOnMouseEvent = true;
            allowFocus = true;
        }

        bool ToggleButton::eventMouseDown(const MouseButton B, const float X, const float Y)  {
            const bool r = CheckWidget::eventMouseDown(B, X, Y);
            if (getRect().contains(X, Y)) {
                auto event = EventClick{};
                emit(Event::OnClick, &event);
                return event.consumed;
            }
            return r;
        }

    }
}
