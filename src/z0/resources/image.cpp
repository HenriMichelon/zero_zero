module;
#include "z0/libraries.h"

module z0;

import :Tools;
import :Image;
import :VirtualFS;

namespace z0 {

    Image::Image(
        const uint32_t width,
        const uint32_t height,
        const string & name):
        Resource{name}, width{width}, height{height} {};

    shared_ptr<Image> Image::createBlankImage() {
        const auto& blankJPEG = createBlankJPG();
        return create(1, 1, blankJPEG.size(), blankJPEG.data(), "Blank");
    }

    shared_ptr<Image> Image::load(const string &filepath, const ImageFormat imageFormat) {
        uint32_t texWidth, texHeight;
        uint64_t imageSize;
        auto *pixels = VirtualFS::loadImage(filepath, texWidth, texHeight, imageSize, imageFormat);
        if (!pixels) die("failed to load texture image!");
        auto image = create(texWidth, texHeight, imageSize, pixels, filepath);
        VirtualFS::destroyImage(pixels);
        return image;
    }

}
