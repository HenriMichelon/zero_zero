#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/resources/image.h"
#include "z0/resources/font.h"
#include "z0/gui/gresource.h"
#include "z0/gui/gstyle.h"
#include "z0/gui/gevent.h"
#include "z0/gui/gwidget.h"
#endif
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
        return true;
    }

}