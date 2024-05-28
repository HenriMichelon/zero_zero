#pragma once

#include "z0/input.h"
#include "z0/gui/gbox.h"

namespace z0 {

    // A clickable GBox
    class GButton: public GBox {
    public:
        GButton();

    protected:
        bool eventMouseUp(MouseButton, int32_t, int32_t) override;
    };

}
