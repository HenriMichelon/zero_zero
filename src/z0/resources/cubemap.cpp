module;
#include <cassert>
#include "z0/libraries.h"

module z0;

import :Constants;
import :Application;
import :Image;
import :Cubemap;
import :VirtualFS;

namespace z0 {

    Cubemap::Cubemap(const uint32_t width,
                    const uint32_t  height,
                    const Type      type,
                    const string &  name):
       Resource{name}, type{type}, width{width}, height{height} { }

    shared_ptr<Cubemap> Cubemap::createBlankCubemap() {
        auto blankJPEG = createBlankJPG();
        const auto cubeFaces = vector{blankJPEG.data(),blankJPEG.data(),blankJPEG.data(),blankJPEG.data(),blankJPEG.data(),blankJPEG.data()};
        return create(1, 1, blankJPEG.size(), cubeFaces);
    }

    shared_ptr<Cubemap> Cubemap::loadFromFile(const string &filepath, const string &fileext, const ImageFormat imageFormat) {
        uint32_t texWidth, texHeight;
        uint64_t imageSize;
        vector<byte *> data;
        const array<std::string, 6> names{"right", "left", "top", "bottom", "front", "back"};
        for (int i = 0; i < 6; i++) {
            auto path = filepath + "_" + names[i] + fileext;
            auto *pixels = VirtualFS::loadImage(path, texWidth, texHeight, imageSize, imageFormat);
            if (!pixels)
                die("failed to load texture image", path);
            data.push_back(pixels);
        }
        const auto &cubemap = create(
            texWidth, texHeight,
            imageSize,
            data);
        for (int i = 0; i < 6; i++) {
            VirtualFS::destroyImage(data[i]);
        }
        return cubemap;
    }

    shared_ptr<Cubemap> Cubemap::loadFromFile(const string &filepath, const ImageFormat imageFormat) {
        assert(imageFormat == IMAGE_R8G8B8A8);
        uint32_t texWidth, texHeight;
        uint64_t imageSize;
        auto *pixels = VirtualFS::loadImage(filepath, texWidth, texHeight, imageSize, imageFormat);
        if (!pixels)
            die("failed to load texture image", filepath);
        vector<byte*> data;
        const auto              imgWidth  = texWidth / 4;
        const auto              imgHeight = texHeight / 3;
        // right
        data.push_back(extractImage(pixels,
                                    2 * imgWidth,
                                    1 * imgHeight,
                                    texWidth,
                                    imgWidth,
                                    imgHeight,
                                    4));
        // left
        data.push_back(extractImage(pixels,
                                    0 * imgWidth,
                                    1 * imgHeight,
                                    texWidth,
                                    imgWidth,
                                    imgHeight,
                                    4));
        // top
        data.push_back(extractImage(pixels,
                                    1 * imgWidth,
                                    0 * imgHeight,
                                    texWidth,
                                    imgWidth,
                                    imgHeight,
                                    4));
        // bottom
        data.push_back(extractImage(pixels,
                                    1 * imgWidth,
                                    2 * imgHeight,
                                    texWidth,
                                    imgWidth,
                                    imgHeight,
                                    4));
        // front
        data.push_back(extractImage(pixels,
                                    1 * imgWidth,
                                    1 * imgHeight,
                                    texWidth,
                                    imgWidth,
                                    imgHeight,
                                    4));
        // back
        data.push_back(extractImage(pixels,
                                    3 * imgWidth,
                                    1 * imgHeight,
                                    texWidth,
                                    imgWidth,
                                    imgHeight,
                                    4));
        const auto &cubemap = create(imgWidth,
                                     imgHeight,
                                     imgWidth * imgHeight * 4,
                                     data);
        for (int i = 0; i < 6; i++) {
            delete[] data[i];
        }
        VirtualFS::destroyImage(pixels);
        return cubemap;
    }

    byte *Cubemap::extractImage(const byte *source,
                                const int   x, const int y,
                                const int   srcWidth,
                                const int   w, const int h,
                                const int   channels) {
        const auto extractedImage = new byte[w * h * channels];
        for (uint32_t row = 0; row < h; ++row) {
            for (uint32_t col = 0; col < w; ++col) {
                for (uint32_t c = 0; c < channels; ++c) {
                    extractedImage[(row * w + col) * channels + c] = source[((y + row) * srcWidth + (x + col)) *
                        channels + c];
                }
            }
        }
        return extractedImage;
    }


}
