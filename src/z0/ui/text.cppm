/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.ui.Text;

import z0.Application;

import z0.ui.Rect;
import z0.ui.Widget;

export namespace z0 {

    namespace ui {
        /**
         * %A widget to display a line of text
         */
        class Text : public Widget {
        public:
            explicit Text(string C): Widget(TEXT), text(std::move(C)) {
                allowChildren = false;
                drawBackground = false;
                textColor = vec4{1.0f};
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

            void setTextColor(const vec4 c) {
                textColor = c;
                refresh();
            }

            [[nodiscard]] vec4 getTextColor() const { return textColor; }

            /*  void setSize(float, float) override;
             void setRect(const Rect&);
             void setRect(float, float, float, float); */

            /**
            * Returns the size (in screen units, VECTOR_SCALE ratio applied) for the text.
            */
            void getSize(float& width, float& height) {
                getFont().getSize(text, width, height);
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
                    Widget::setSize(width, height);
                }
            }

        protected:
            [[nodiscard]] Rect _getDefaultRect() override {
                if (rect.width == 0 && rect.height == 0) {
                    float w, h;
                    getSize(w, h);
                    setSize(w, h);
                }
                return Widget::_getDefaultRect();
            }

        private:
            string text;
            vec4   textColor;

            /* void computeSize();*/
            void eventCreate() override {
                getSize(rect.width, rect.height);
                Widget::eventCreate();
            }
        };
    }
}
