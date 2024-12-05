/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

module z0.resources.Texture;

import z0.Constants;

import z0.resources.Image;

namespace z0 {

    Texture::Texture(const string &name):
        Resource{name} {
    }

    ImageTexture::ImageTexture(const shared_ptr<Image> &img):
        Texture{img->getName()}, image{img} {
    }

    ImageTexture::ImageTexture(const string &filename, const ImageFormat imageFormat):
        Texture{filename}, image{Image::load(filename, imageFormat)} {
    }

}
