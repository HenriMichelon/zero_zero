/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.GText;

import z0.Color;
import z0.Rect;
import z0.Application;
import z0.GWidget;

export namespace z0 {
    /**
     * %A widget to display a line of text
     */
    class GText : public GWidget {
    public:
        explicit GText(string C): GWidget(TEXT), text(std::move(C)) {
            allowChildren = false;
            drawBackground = false;
        }

        [[nodiscard]] const string& getText() const { return text; }

        void setText(const string& C) {
            text = C;
            if (parent) { parent->refresh(); }
            float w, h;
            getSize(w, h);
            setSize(w, h);
            if (!parent) { refresh(); }
        }

        void setTextColor(Color c) {
            textColor = c;
            textColor.color.a = 1.0f;
            refresh();
        }

        [[nodiscard]] Color getTextColor() const { return textColor; }

        /*  void setSize(float, float) override;
         void setRect(const Rect&);
         void setRect(float, float, float, float); */

        /**
        * Returns the size (in screen units, VECTOR_SCALE ratio applied) for the text.
        */
        void getSize(float& width, float& height) {
            getFont()->getSize(text, width, height);
            const auto& ratio = Application::get().getVectorRatio();
            width = roundf(width / ratio.x);
            height = roundf(height / ratio.y);
        }

        void setSize(const float width, const float height) override {
            if (width == 0 && height == 0 && rect.width == 0 && rect.height == 0) {
                float w, h;
                getSize(w, h);
                setSize(w, h);
            }
            else {
                GWidget::setSize(width, height);
            }
        }

    protected:
        [[nodiscard]] Rect _getDefaultRect() override {
            if (rect.width == 0 && rect.height == 0) {
                float w, h;
                getSize(w, h);
                setSize(w, h);
            }
            return GWidget::_getDefaultRect();
        }

    private:
        string text;
        Color textColor;

        /* void computeSize();*/
        void eventCreate() override {
            getSize(rect.width, rect.height);
            GWidget::eventCreate();
        }
    };
}
