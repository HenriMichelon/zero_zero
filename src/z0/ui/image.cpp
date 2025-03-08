/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

module z0.ui.Image;

import z0.Application;
import z0.Log;

namespace z0::ui {

    Image::Image(const shared_ptr<z0::Image> &image, const bool autoSize) : Widget{IMAGE}, autoSize{autoSize} {
        setImage(image);
    }

    void Image::_setSize(const float width, const float height) {
        if (autoSize) { return; }
        if (width == 0 && height == 0 && rect.width == 0 && rect.height == 0) {
            const auto &ratio = app().getVectorRatio();
            Widget::_setSize(round(width / ratio), round(height / 1.0f));
        }
        else {
            Widget::_setSize(width, height);
        }
    }

    void Image::autoResize() {
        const auto &ratio = app().getVectorRatio();
        Widget::_setSize(round(image->getWidth() / ratio), round(image->getHeight() / 1.0f));
    }

    void Image::setColor(const vec4 &color) {
        this->color = color;
        refresh();
    }


    void Image::setAutoSize(const bool autoSize) {
        if (autoSize == this->autoSize) {
            return;
        }
        this->autoSize = autoSize;
        if (autoSize && image) {
            this->autoResize();
        }
    }

    void Image::setImage(const shared_ptr<z0::Image> &image) {
        if (this->image == image) {
            return;
        }
        this->image = image;
        if (image) {
            if (autoSize) {
                autoResize();
            } else {
                refresh();
            }
        }
    }

} // namespace z0::ui
