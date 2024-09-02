module;
#include "z0/libraries.h"

export module Z0:GToggleButton;

import :Constants;
import :GEvent;
import :GCheckWidget;

export namespace z0 {

    /**
     * Two states clickeable button
     */
    class GToggleButton: public GCheckWidget {
    public:
        GToggleButton();
        virtual ~GToggleButton() {};

    protected:
        virtual bool eventMouseDown(MouseButton, float, float);
    };

    GToggleButton::GToggleButton(): GCheckWidget(TOGGLEBUTTON) {
        moveChildrenOnPush = true;
        redrawOnMouseEvent = true;
        allowFocus = true;
    }

    bool GToggleButton::eventMouseDown(MouseButton B, float X, float Y) {
        bool r = GCheckWidget::eventMouseDown(B, X, Y);
        if (getRect().contains(X, Y)) {
            auto event = GEventClick{};
            emit(GEvent::OnClick, &event);
            return event.consumed;
        }
        return r;
    }


}