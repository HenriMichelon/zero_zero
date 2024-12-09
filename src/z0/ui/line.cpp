/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;

module z0.ui.Line;

namespace z0 {

    namespace ui {

        Line::Line(const LineStyle K): Widget{LINE}, style{K} {
            allowChildren = false;
        }

        void Line::setStyle(const LineStyle K) {
            if (style != K) {
                style = K;
                resizeChildren();
                refresh();
            }
        }

    }
}
