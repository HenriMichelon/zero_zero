#pragma once

#include "z0/gui/gwidget.h"

namespace z0 {

    // A rectangular widget with only a background (no borders)
    class GPanel: public GWidget {
    public:
        GPanel(): GWidget(PANEL) {};

    protected:
        explicit GPanel(Type T): GWidget(T) {};
    };

}