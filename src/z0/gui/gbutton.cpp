#include "z0/gui/gbutton.h"

namespace z0 {

    GButton::GButton(): GBox{BUTTON} {
        moveChildsOnPush = true;
        redrawOnMouseEvent = true;
        redrawOnMouseMove = true;
        allowFocus = true;
    }

    bool GButton::eventMouseUp(MouseButton B, uint32_t X, uint32_t Y) {
        const bool p = isPushed();
        bool consumed = GBox::eventMouseUp(B, X, Y);
        if ((!consumed) && p) {
            if (call(GEvent::OnClick)) { return true; }
        }
        return consumed;
    }

}