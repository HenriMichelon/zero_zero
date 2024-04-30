#include "z0/resources/texture.h"

namespace z0 {

    ImageTexture::ImageTexture(const string& filename):
        Texture{filename},
        image{Image::loadFromFile(filename)} {
    }

}