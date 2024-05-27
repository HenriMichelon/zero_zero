#include <utility>

#include "z0/gui/gtext.h"

namespace z0 {

    GText::GText(string C): GPanel(TEXT), text(std::move(C)) {
        allowChildren = false;
        drawBackground = false;
    }

    void GText::setTextColor(Color c) {
        textColor = c;
        textColor.color.a = 1.0f;
        refresh();
    }

    void GText::setText(const string &C) {
        text = C;
        if (parent) { parent->refresh(); }
        computeSize();
        if (!parent) { refresh(); }
        call(GEvent::OnTextChange);
    }

    void GText::setAutoSize(bool A) {
        if (autoSize == A) return;
        autoSize = A;
        computeSize();
    }

    void GText::eventCreate() {
        computeSize();
        GPanel::eventCreate();
    }

    void GText::computeSize() {
        if (!text.empty()) {
            uint32_t w, h;
            getFont()->getSize(text, w, h);
            if (autoSize) {
                setSize(w, h);
            } else {
                setFreezed(true);
                rect.height = h;
                resizeChildren();
                setFreezed(false);
            }
        }
    }

}
