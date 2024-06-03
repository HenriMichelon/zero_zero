#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/resources/image.h"
#include "z0/resources/texture.h"
#endif

namespace z0 {

    ImageTexture::ImageTexture(const string& filename):
        Texture{filename},
        image{Image::loadFromFile(filename)} {
    }

}