
#include "z0/gui/gcheck_widget.h"

namespace z0 {

    GCheckWidget::GCheckWidget(Type T): GWidget{T} {}

    bool GCheckWidget::eventMouseDown(MouseButton B, uint32_t X, uint32_t Y) {
        if (rect.contains(X, Y)) {
            if (state == CHECK) {
                setState(UNCHECK);
            } else {
                setState(CHECK);
            }
        }
        return GWidget::eventMouseDown(B, X, Y);
    }

    void GCheckWidget::setState(CheckState S) {
        if (state == S) { return; }
        state = S;
        resizeChildren();
        refresh();
        auto stat = make_shared<GEventState>(S);
        call(GEvent::OnStateChange, stat);
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