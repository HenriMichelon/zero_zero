module;

export module Z0:GButton;

import :Constants;
import :GBox;
import :GEvent;

export namespace z0 {
    /**
     * A clickable GBox
     */
    class GButton : public GBox {
    public:
        GButton(): GBox{BUTTON} {
            moveChildrenOnPush = true;
            redrawOnMouseEvent = true;
            allowFocus = true;
        }

    protected:
        bool eventMouseUp(const MouseButton B, const float X, const float Y) override {
            const bool p = isPushed();
            if (p && (!getRect().contains(X, Y))) {
                setPushed(false);
                resizeChildren();
                return GBox::eventMouseUp(B, X, Y);
            }
            const bool consumed = GBox::eventMouseUp(B, X, Y);
            if ((!consumed) && p) {
                auto event = GEventClick{};
                emit(GEvent::OnClick, &event);
                return event.consumed;
            }
            return consumed;
        }
    };
}
