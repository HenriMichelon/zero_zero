module;
#include "z0/libraries.h"

export module Z0:GButton;

import :Constants;
import :GBox;
import :GEvent;

export namespace z0 {

    /**
     * A clickable GBox
     */
    class GButton: public GBox {
    public:
        GButton(): GBox{BUTTON} {
            moveChildrenOnPush = true;
            redrawOnMouseEvent = true;
            allowFocus = true;
        }

    protected:
        bool eventMouseUp(MouseButton B, float X, float Y) override {
        const bool p = isPushed();
        if (p && (!getRect().contains(X, Y))) {
            setPushed(false);
            resizeChildren();
            return GBox::eventMouseUp(B, X, Y);
        }
        bool consumed = GBox::eventMouseUp(B, X, Y);
        if ((!consumed) && p) {
            auto event = GEventClick{ };
            emit(GEvent::OnClick, &event);
            return event.consumed;
        }
        return consumed;
    }
    };

}
