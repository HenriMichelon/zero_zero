#include "z0/gui/gbutton.h"

namespace z0 {

    GButton::GButton(): GBox{BUTTON} {
        moveChildsOnPush = true;
        redrawOnMouseEvent = true;
        //redrawOnMouseMove = true;
        allowFocus = true;
    }

    bool GButton::eventMouseUp(MouseButton B, int32_t X, int32_t Y) {
        const bool p = isPushed();
        if (p && (!rect.contains(X, Y))) {
            setPushed(false);
            resizeChildren();
            return GBox::eventMouseUp(B, X, Y);
        }
        bool consumed = GBox::eventMouseUp(B, X, Y);
        if ((!consumed) && p) {
            if (call(GEvent::OnClick)) { return true; }
        }
        return consumed;
    }

}