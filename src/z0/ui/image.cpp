/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

module z0.ui.Image;

import z0.Application;

namespace z0 {
    namespace ui {

        Image::Image(const shared_ptr<z0::Image> &image, const bool autoSize):
            Widget{IMAGE},
            autoSize{autoSize} {
            setImage(image);
        }

        void Image::autoResize() {
            const auto& ratio = Application::get().getVectorRatio();
            setSize(round(image->getWidth() / ratio.x), round(image->getHeight() / ratio.y));
        }

        void Image::setColor(const vec4 &color) {
            this->color = color;
            refresh();
        }

        void Image::setAutoSize(const bool autoSize) {
            if (autoSize == this->autoSize) { return; }
            this->autoSize = autoSize;
            if (autoSize && image) { this->autoResize(); }
        }

        void Image::setImage(const shared_ptr<z0::Image>& image) {
            if (this->image == image) { return; }
            this->image = image;
            if (image) {
                if (autoSize) {
                    autoResize();
                }
                else {
                    refresh();
                }
            }
        }

    }
}
