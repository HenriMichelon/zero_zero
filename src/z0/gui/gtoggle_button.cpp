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
#include "z0/gui/gcheck_widget.h"
#include "z0/gui/gtoggle_button.h"
#endif

namespace z0 {

    GToggleButton::GToggleButton(): GCheckWidget(TOGGLEBUTTON) {
        moveChildsOnPush = true;
        redrawOnMouseEvent = true;
        allowFocus = true;
    }

    bool GToggleButton::eventMouseDown(MouseButton B, float X, float Y) {
        bool r = GCheckWidget::eventMouseDown(B, X, Y);
        if (getRect().contains(X, Y)) {
            if (call(GEvent::OnClick)) { return true; }
        }
        return r;
    }
}