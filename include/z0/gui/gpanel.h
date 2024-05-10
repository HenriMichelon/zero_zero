#pragma once

#include "z0/gui/gwidget.h"

namespace z0 {

    class GPanel: public GWidget {
    public:
        GPanel(): GWidget(PANEL) {};

    protected:
        explicit GPanel(Type T): GWidget(T) {};
    };

}