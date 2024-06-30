#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/resources/image.h"
#include "z0/resources/font.h"
#include "z0/nodes/node.h"
#include "z0/application.h"
#include "z0/gui/gresource.h"
#include "z0/gui/gstyle.h"
#include "z0/gui/gevent.h"
#include "z0/gui/gwidget.h"
#endif
#include "z0/gui/gpanel.h"
#include "z0/gui/gframe.h"

namespace z0 {
    GFrame::GFrame(const string& STR): GPanel(GWidget::FRAME), text(STR) {
    }

    void GFrame::setText(const string &C) {
        text = C;
        resizeChildren();
        refresh();
    }


}
