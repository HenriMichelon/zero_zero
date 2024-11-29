/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.ui.CheckWidget;

import z0.Constants;

import z0.ui.Event;
import z0.ui.Widget;

export namespace z0 {
    namespace ui {
        /**
         * Super class for all two-states widgets
         */
        class CheckWidget : public Widget {
        public:
            //! State of the widget. Change on user action.
            enum State {
                //! Checked (aka ON)
                CHECK,
                //! Unchecked (aka OFF)
                UNCHECK
            };

            //! Return current state of the widget
            [[nodiscard]] State getState() const { return state; }

            //! Change the state of the widget
            virtual void setState(const State S) {
                if (state == S) { return; }
                state = S;
                resizeChildren();
                refresh();
                auto stat = GEventState{.state = S};
                emit(Event::OnStateChange, &stat);
            }

        protected:
            explicit CheckWidget(const Type T): Widget{T} {
            }

            bool eventMouseDown(const MouseButton B, const float X, const float Y) override {
                if (getRect().contains(X, Y)) {
                    if (state == CHECK) {
                        setState(UNCHECK);
                    }
                    else {
                        setState(CHECK);
                    }
                }
                return Widget::eventMouseDown(B, X, Y);
            }

        private:
            State state{UNCHECK};
        };
    }

}
