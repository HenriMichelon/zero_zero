#pragma once

#include "z0/gui/gpanel.h"

namespace z0 {

    /**
     * A rectangular box
     */
    class GBox: public GPanel {
    public:
        GBox();

    protected:
        GBox(Type);
    };


}