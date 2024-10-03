module;
#include "z0/libraries.h"

module z0;

import :Texture;
import :Image;

namespace z0 {

    Texture::Texture(const string &name):
        Resource{name} {
    }

    ImageTexture::ImageTexture(const shared_ptr<Image> &img):
        Texture{img->getName()}, image(img) {
    }

    ImageTexture::ImageTexture(const string &filename):
        Texture{filename},
        image{Image::loadFromFile(filename)} {
    }

}
