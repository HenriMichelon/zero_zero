/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.ui.Text;

import z0.ui.Rect;
import z0.ui.Widget;

export namespace z0 {

    namespace ui {
        /**
         * %A widget to display a line of text
         */
        class Text : public Widget {
        public:
            explicit Text(string C);

            [[nodiscard]] const string& getText() const { return text; }

            void setText(const string& C);

            void setTextColor(const vec4& c);

            [[nodiscard]] vec4 getTextColor() const { return textColor; }

            /*  void _setSize(float, float) override;
             void setRect(const Rect&);
             void setRect(float, float, float, float); */

            /**
            * Returns the size (in screen units, VECTOR_SCALE ratio applied) for the text.
            */
            void getSize(float& width, float& height);

            void _setSize(float width, float height) override;

        protected:
            [[nodiscard]] Rect _getDefaultRect() override;

        private:
            string text;
            vec4   textColor{0.0f};

            /* void computeSize();*/
            void eventCreate() override;
        };
    }
}
