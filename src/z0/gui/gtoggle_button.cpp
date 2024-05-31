#include "z0/base.h"
#include "z0/gui/gcheck_widget.h"
#include "z0/gui/gtoggle_button.h"

namespace z0 {

    GToggleButton::GToggleButton(): GCheckWidget(TOGGLEBUTTON) {
        moveChildsOnPush = true;
        redrawOnMouseEvent = true;
        allowFocus = true;
    }

    bool GToggleButton::eventMouseDown(MouseButton B, uint32_t X, uint32_t Y) {
        bool r = GCheckWidget::eventMouseDown(B, X, Y);
        if (rect.contains(X, Y)) {
            if (call(GEvent::OnClick)) { return true; }
        }
        return r;
    }
}