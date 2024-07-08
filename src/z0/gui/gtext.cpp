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
        float w, h;
        getSize(w, h);
        setSize(w, h);
        if (!parent) { refresh(); }
    }

    void GText::setSize(float width, float height) {
        if (width == 0 && height == 0 && rect.width == 0 && rect.height == 0) {
            float w, h;
            getSize(w, h);
            setSize(w, h);
        } else {
            GWidget::setSize(width, height);
        }
    }

    Rect GText::_getDefaultRect() {
        if (rect.width == 0 && rect.height == 0) {
            float w, h;
            getSize(w, h);
            setSize(w, h);
        }
        return GWidget::_getDefaultRect();
    }

    void GText::getSize(float&width, float&height) {
        getFont()->getSize(text, width, height);
        const auto& ratio = Application::get().getVectorRatio();
        width = roundf(width / ratio.x);
        height = roundf(height / ratio.y);
    }
 
    void GText::eventCreate() {
        getSize(rect.width, rect.height);
        GWidget::eventCreate();
    }
/*
    void GText::setSize(float w, float h) {
    }

    void GText::setRect(const Rect&) {
    }

    void GText::setRect(float, float, float, float) {
    }

    void GText::computeSize() {
        if (!text.empty()) {
            getFont()->getSize(text, rect.width, rect.height);
            const auto& ratio = Application::get().getVectorRatio();
            rect.width = roundf(rect.width / ratio.x);
            rect.height = roundf(rect.height / ratio.y);
        }
    } */

}
