#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/resources/image.h"
#include "z0/resources/font.h"
#include "z0/nodes/node.h"
#include "z0/gui/gresource.h"
#include "z0/gui/gstyle.h"
#include "z0/gui/gevent.h"
#include "z0/gui/gwidget.h"
#include "z0/gui/gtext.h"
#include "z0/application.h"
#endif

namespace z0 {

    GText::GText(string C): GWidget(TEXT), text(std::move(C)) {
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

    void GText::eventCreate() {
        computeSize();
        GWidget::eventCreate();
    }

    void GText::setSize(float w, float h) {
    }

    void GText::computeSize() {
        if (!text.empty()) {
            getFont()->getSize(text,rect.width, rect.height);
        }
    }

}
