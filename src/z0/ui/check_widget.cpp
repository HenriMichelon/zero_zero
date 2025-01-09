/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;

module z0.ui.CheckWidget;

import z0.ui.Event;

namespace z0 {
    namespace ui {

        void CheckWidget::setState(const State S) {
            if (state == S) { return; }
            state = S;
            resizeChildren();
            refresh();
            auto stat = EventState{.state = S};
            emit(Event::OnStateChange, &stat);
        }

        bool CheckWidget::eventMouseDown(const MouseButton B, const float X, const float Y) {
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

    }

}
