#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/resources/image.h"
#include "z0/resources/font.h"
#include "z0/nodes/node.h"
#include "z0/application.h"
#include "z0/gui/gresource.h"
#include "z0/gui/gstyle.h"
#include "z0/gui/gevent.h"
#include "z0/gui/gwidget.h"
#endif
#include "z0/gui/gbutton.h"

namespace z0 {

    GButton::GButton(): GBox{BUTTON} {
        moveChildrenOnPush = true;
        redrawOnMouseEvent = true;
        //redrawOnMouseMove = true;
        allowFocus = true;
    }

    bool GButton::eventMouseUp(MouseButton B, float X, float Y) {
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

}