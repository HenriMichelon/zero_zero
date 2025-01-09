/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.ui.Frame;

import z0.ui.Panel;

export namespace z0 {

    namespace ui {

        /**
         * %A rectangular frame with an optional title
         */
        class Frame: public Panel {
        public:
            // Create a Frame widget with an optional title
            explicit Frame(const string& STR= ""): Panel(Widget::FRAME), text(STR) {}

            // Return the current title of the widget
            [[nodiscard]] inline const string& getText() const { return text; }

            // Change the title of the widget
            void setText(const string& T);

            void setTextColor(const vec4 c) { textColor = c; }

            [[nodiscard]] vec4 getTextColor() const { return textColor; }

        private:
            string text{};
            vec4   textColor{1.0f};
        };
    }

}
