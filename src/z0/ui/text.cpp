/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

module z0.ui.Text;

import z0.Application;

namespace z0 {
    namespace ui {

        Text::Text(string C): Widget(TEXT), text(std::move(C)) {
            allowChildren = false;
            drawBackground = false;
            textColor = vec4{0.0f, 0.0f, 0.0f, 1.0f};
        }

        void Text::setText(const string& C) {
            text = C;
            if (parent) { parent->refresh(); }
            float w, h;
            getSize(w, h);
            setSize(w, h);
            if (!parent) { refresh(); }
        }

        void Text::setTextColor(const vec4& c) {
            textColor = c;
            refresh();
        }

        void Text::getSize(float& width, float& height) {
            getFont().getSize(text, width, height);
            const auto& ratio = app().getVectorRatio();
            width = roundf(width / ratio.x);
            height = roundf(height / ratio.y);
        }

        void Text::setSize(const float width, const float height) {
            if (width == 0 && height == 0 && rect.width == 0 && rect.height == 0) {
                float w, h;
                getSize(w, h);
                setSize(w, h);
            }
            else {
                Widget::setSize(width, height);
            }
        }

        [[nodiscard]] Rect Text::_getDefaultRect() {
            if (rect.width == 0 && rect.height == 0) {
                float w, h;
                getSize(w, h);
                setSize(w, h);
            }
            return Widget::_getDefaultRect();
        }

        void Text::eventCreate()  {
            getSize(rect.width, rect.height);
            Widget::eventCreate();
        }
    }
}
