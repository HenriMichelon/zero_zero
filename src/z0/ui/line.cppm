/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;

export module z0.ui.Line;

import z0.ui.Widget;

export namespace z0 {

    namespace ui {

        /**
         * %A horizontal or vertical line
         */
        class Line : public Widget {
        public:
            enum LineStyle {
                HORIZ,
                VERT
            };

            explicit Line(LineStyle K = HORIZ);

            [[nodiscard]] inline LineStyle getStyle() const { return style; }

            void setStyle(LineStyle K);

        private:
            LineStyle style;
        };

        class HLine : public Line {
        public:
            HLine(): Line(HORIZ) {}
        };

        class VLine : public Line {
        public:
            VLine(): Line(VERT) {}
        };
    }
}
