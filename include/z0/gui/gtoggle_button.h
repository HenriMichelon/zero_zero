#pragma once

#include "z0/gui/gcheck_widget.h"

namespace z0 {

    class GToggleButton: public GCheckWidget {
    public:
        GToggleButton();
        virtual ~GToggleButton() {};

    protected:
        virtual bool eventMouseDown(MouseButton, uint32_t, uint32_t);
    };


}