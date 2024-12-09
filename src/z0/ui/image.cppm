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
            explicit Image(const shared_ptr<z0::Image> &image = nullptr);

            void setAutoSize(bool autoSize);

            void setImage(const shared_ptr<z0::Image>& image);

            shared_ptr<z0::Image> getImage() const { return image; }

        private:
            bool                  autoSize{true};
            shared_ptr<z0::Image> image;

            void autoResize();

        };
    }
}
