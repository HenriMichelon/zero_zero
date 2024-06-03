#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/resources/image.h"
#include "z0/resources/font.h"
#include "z0/gui/gresource.h"
#include "z0/gui/gstyle.h"
#include "z0/gui/gevent.h"
#include "z0/gui/gwidget.h"
#endif
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