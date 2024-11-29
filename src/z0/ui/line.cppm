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

            explicit Line(const LineStyle K = HORIZ): Widget{LINE}, style{K} {
                allowChildren = false;
            }

            [[nodiscard]] LineStyle getStyle() const { return style; }

            void setStyle(const LineStyle K) {
                if (style != K) {
                    style = K;
                    resizeChildren();
                    refresh();
                }
            }

        private:
            LineStyle style;
        };

        class HLine : public Line {
        public:
            HLine(): Line(HORIZ) {
            }
        };

        class VLine : public Line {
        public:
            VLine(): Line(VERT) {
            }
        };
    }
}
