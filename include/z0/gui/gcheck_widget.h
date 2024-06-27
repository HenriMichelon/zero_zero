#pragma once

#include "z0/gui/gwidget.h"

namespace z0 {

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

    
}