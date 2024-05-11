#include "z0/gui/gline.h"

namespace z0 {

    GLine::GLine(GLine::LineStyle K): GWidget{LINE}, style{K} {
        allowChildren = false;
    }

    void GLine::setStyle(GLine::LineStyle K) {
        if (style != K)	{
            style = K;
            resizeChildren();
            refresh();
        }
    }

}