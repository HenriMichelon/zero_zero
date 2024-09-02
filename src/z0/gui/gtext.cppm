module;
#include "z0/modules.h"

export module Z0:GText;

import :Color;
import :Rect;
import :Application;
import :GWidget;

export namespace z0 {

    /**
     * A widget to display a line of text
     */ 
    class GText: public GWidget {
    public:
        explicit GText(string);

        [[nodiscard]] const string& getText() const { return text; }
        void setText(const string&);
        void setTextColor(Color c);
        [[nodiscard]] Color getTextColor() const { return textColor; }

       /*  void setSize(float, float) override;
        void setRect(const Rect&);
        void setRect(float, float, float, float); */

         /**
         * Returns the size (in screen units, VECTOR_SCALE ratio applied) for the text.
         */
        void getSize(float&width, float&height);

        void setSize(float width, float height) override;

    protected:
        [[nodiscard]] Rect _getDefaultRect() override;

    private:
        string	text;
        Color   textColor;

        /* void computeSize();*/
        void eventCreate() override;

    };

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
