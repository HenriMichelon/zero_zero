module;
#include "z0/libraries.h"

export module z0:GToggleButton;

import :Constants;
import :GEvent;
import :GCheckWidget;

export namespace z0 {
    /**
     * Two states clickable button
     */
    class GToggleButton : public GCheckWidget {
    public:
        GToggleButton(): GCheckWidget(TOGGLEBUTTON) {
            moveChildrenOnPush = true;
            redrawOnMouseEvent = true;
            allowFocus = true;
        }

        ~GToggleButton() override {
        };

    protected:
        bool eventMouseDown(const MouseButton B, const float X, const float Y) override {
            const bool r = GCheckWidget::eventMouseDown(B, X, Y);
            if (getRect().contains(X, Y)) {
                auto event = GEventClick{};
                emit(GEvent::OnClick, &event);
                return event.consumed;
            }
            return r;
        }
    };
}
