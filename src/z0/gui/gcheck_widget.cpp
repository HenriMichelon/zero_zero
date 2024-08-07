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
#include "z0/gui/gcheck_widget.h"

namespace z0 {

    GCheckWidget::GCheckWidget(Type T): GWidget{T} {}

    bool GCheckWidget::eventMouseDown(MouseButton B, uint32_t X, uint32_t Y) {
        if (getRect().contains(X, Y)) {
            if (state == CHECK) {
                setState(UNCHECK);
            } else {
                setState(CHECK);
            }
        }
        return GWidget::eventMouseDown(B, X, Y);
    }

    void GCheckWidget::setState(State S) {
        if (state == S) { return; }
        state = S;
        resizeChildren();
        refresh();
        auto stat = GEventState{ .state = S};
        emit(GEvent::OnStateChange, &stat);
    }

/*//----------------------------------------------
    GCheckButton::GCheckButton(): GCheckWidget(GWidget::CHECKBUTTON)
    {
    }


//----------------------------------------------
    void GCheckButton::EventCreate()
    {
        Add(box, CLIENT, resource->Resource());
        box.Add(checkmark, CLIENT, resource->Resource());
        allowChilds = FALSE;
        checkmark.Show(State() == CHECK);
        GCheckWidget::EventCreate();
    }


//----------------------------------------------
    void GCheckButton::SetState(CheckState S)
    {
        checkmark.Show(State() == UNCHECK);
        GCheckWidget::SetState(S);
    }
*/

}