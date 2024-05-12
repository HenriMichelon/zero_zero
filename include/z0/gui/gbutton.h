#pragma once

#include "z0/input.h"
#include "z0/gui/gbox.h"

namespace z0 {

    class GButton: public GBox {
    public:
        GButton();

    protected:
        bool eventMouseUp(MouseButton, uint32_t, uint32_t) override;
    };

}
