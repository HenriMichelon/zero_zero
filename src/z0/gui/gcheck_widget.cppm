/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0:GCheckWidget;

import :Constants;
import :GWidget;
import :GEvent;

export namespace z0 {
    /**
     * Super class for all two-states widgets
     */
    class GCheckWidget : public GWidget {
    public:
        //! State of the widget. Change on user action.
        enum State {
            CHECK, //! Checked (aka ON)
            UNCHECK //! Unchecked (aka OFF)
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
            emit(GEvent::OnStateChange, &stat);
        }

    protected:
        explicit GCheckWidget(const Type T): GWidget{T} {
        }

        virtual bool eventMouseDown(const MouseButton B, const uint32_t X, const uint32_t Y) {
            if (getRect().contains(X, Y)) {
                if (state == CHECK) {
                    setState(UNCHECK);
                }
                else {
                    setState(CHECK);
                }
            }
            return GWidget::eventMouseDown(B, X, Y);
        }

    private:
        State state{UNCHECK};
    };

}
