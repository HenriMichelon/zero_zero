#include "z0/gui/gframe.h"

namespace z0 {
    GFrame::GFrame(const string& STR): GPanel(GWidget::FRAME), text(STR) {
    }

    void GFrame::setText(const string &C) {
        text = C;
        resizeChildren();
        refresh();
        call(GEvent::OnTextChange);
    }


}
