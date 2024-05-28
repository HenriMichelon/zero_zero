#pragma once

#include "z0/gui/gpanel.h"

namespace z0 {

    // a rectangular box
    class GBox: public GPanel {
    public:
        GBox();

    protected:
        GBox(Type);
    };


}