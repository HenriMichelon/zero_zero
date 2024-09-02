module;
#include "z0/modules.h"

export module Z0:GCheckWidget;

import :Constants;
import :GWidget;
import :GEvent;

export namespace z0 {

    /**
     * Super class for all two-states widgets
     */
    class GCheckWidget: public GWidget {
    public:
        //! State of the widget. Change on user action.
        enum State {
            CHECK,		//! Checked (aka ON)
            UNCHECK		//! Unchecked (aka OFF)
        } ;

        //! Return current state of the widget
        [[nodiscard]] State getState() const { return state; }

        //! Change the state of the widget
        virtual void setState(State);

    protected:
        GCheckWidget(Type);

        virtual bool eventMouseDown(MouseButton, uint32_t, uint32_t);

    private:
        State	state{UNCHECK};
    };

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