/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.ui.Image;

import z0.ui.Rect;
import z0.ui.Widget;

import z0.resources.Image;

export namespace z0 {

    namespace ui {

        /**
         * %A widget to display a 2D image
         */
        class Image : public Widget {
        public:
            explicit Image(const shared_ptr<z0::Image> &image = nullptr, bool autoSize = true);

            void setAutoSize(bool autoSize);

            void setColor(const vec4& color);

            vec4 getColor() const { return color; };

            void setImage(const shared_ptr<z0::Image>& image);

            shared_ptr<z0::Image> getImage() const { return image; }

        private:
            vec4                  color{1.0f};
            bool                  autoSize;
            shared_ptr<z0::Image> image;

            void autoResize();

        };
    }
}
