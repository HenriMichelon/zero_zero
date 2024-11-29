/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.ui.ToggleButton;

import z0.Constants;

import z0.ui.CheckWidget;
import z0.ui.Event;

export namespace z0 {

    namespace ui {
        /**
         * Two states clickable button
         */
        class ToggleButton : public CheckWidget {
        public:
            ToggleButton(): CheckWidget(TOGGLEBUTTON) {
                moveChildrenOnPush = true;
                redrawOnMouseEvent = true;
                allowFocus = true;
            }

            ~ToggleButton() override {
            };

        protected:
            bool eventMouseDown(const MouseButton B, const float X, const float Y) override {
                const bool r = CheckWidget::eventMouseDown(B, X, Y);
                if (getRect().contains(X, Y)) {
                    auto event = GEventClick{};
                    emit(Event::OnClick, &event);
                    return event.consumed;
                }
                return r;
            }
        };
    }
}
