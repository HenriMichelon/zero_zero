module;
#include <stb_image.h>
#include <stb_image_write.h>
#include "z0/libraries.h"

module z0;

import :Application;
import :Image;
import :Cubemap;

namespace z0 {

    void cm_stb_write_func(void *context, void *data, const int size) {
        auto *buffer = static_cast<vector<unsigned char> *>(context);
        auto *ptr    = static_cast<unsigned char *>(data);
        buffer->insert(buffer->end(), ptr, ptr + size);
    }

    Cubemap::Cubemap(const uint32_t width,
                    const uint32_t  height,
                    const Type      type,
                    const string &  name):
       Resource{name}, type{type}, width{width}, height{height} { }

    shared_ptr<Cubemap> Cubemap::createBlankCubemap() {
        vector<unsigned char> blankJPEG;
        const auto data = new unsigned char[1 * 1 * 3];
        data[0]   = 0;
        data[1]   = 0;
        data[2]   = 0;
        stbi_write_jpg_to_func(cm_stb_write_func, &blankJPEG, 1, 1, 3, data, 100);
        delete[] data;
        const auto cubeFaces = vector{blankJPEG.data(),blankJPEG.data(),blankJPEG.data(),blankJPEG.data(),blankJPEG.data(),blankJPEG.data()};
        return create(1, 1, blankJPEG.size(), cubeFaces);
    }

    shared_ptr<Cubemap> Cubemap::loadFromFile(const string &filename, const string &fileext) {
        const auto &            filepath = (Application::get().getConfig().appDir / filename).string();
        int                     texWidth, texHeight, texChannels;
        vector<unsigned char *> data;
        const array<std::string, 6> names{"right", "left", "top", "bottom", "front", "back"};
        for (int i = 0; i < 6; i++) {
            string   path   = filepath + "_" + names[i] + fileext;
            stbi_uc *pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            if (!pixels) {
                die("failed to load texture image", path);
            }
            data.push_back(pixels);
        }
        const auto &cubemap = create(
            texWidth, texHeight,
            texWidth * texHeight * STBI_rgb_alpha,
            data);
        for (int i = 0; i < 6; i++) {
            stbi_image_free(data[i]);
        }
        return cubemap;
    }

    shared_ptr<Cubemap> Cubemap::loadFromFile(const string &filename) {
        const auto &filepath = (Application::get().getConfig().appDir / filename).string();
        int         texWidth, texHeight, texChannels;
        stbi_uc *   pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if (!pixels) {
            die("failed to load texture image", filepath);
        }
        vector<unsigned char *> data;
        const auto              imgWidth  = texWidth / 4;
        const auto              imgHeight = texHeight / 3;
        // right
        data.push_back(extractImage(pixels,
                                    2 * imgWidth,
                                    1 * imgHeight,
                                    texWidth,
                                    imgWidth,
                                    imgHeight,
                                    STBI_rgb_alpha));
        // left
        data.push_back(extractImage(pixels,
                                    0 * imgWidth,
                                    1 * imgHeight,
                                    texWidth,
                                    imgWidth,
                                    imgHeight,
                                    STBI_rgb_alpha));
        // top
        data.push_back(extractImage(pixels,
                                    1 * imgWidth,
                                    0 * imgHeight,
                                    texWidth,
                                    imgWidth,
                                    imgHeight,
                                    STBI_rgb_alpha));
        // bottom
        data.push_back(extractImage(pixels,
                                    1 * imgWidth,
                                    2 * imgHeight,
                                    texWidth,
                                    imgWidth,
                                    imgHeight,
                                    STBI_rgb_alpha));
        // front
        data.push_back(extractImage(pixels,
                                    1 * imgWidth,
                                    1 * imgHeight,
                                    texWidth,
                                    imgWidth,
                                    imgHeight,
                                    STBI_rgb_alpha));
        // back
        data.push_back(extractImage(pixels,
                                    3 * imgWidth,
                                    1 * imgHeight,
                                    texWidth,
                                    imgWidth,
                                    imgHeight,
                                    STBI_rgb_alpha));
        const auto &cubemap = create(imgWidth,
                                     imgHeight,
                                     imgWidth * imgHeight * STBI_rgb_alpha,
                                     data);
        for (int i = 0; i < 6; i++) {
            delete[] data[i];
        }
        stbi_image_free(pixels);
        return cubemap;
    }

    unsigned char *Cubemap::extractImage(unsigned char *source,
                                         int            x, int y,
                                         int            srcWidth,
                                         int            w, int h,
                                         int            channels) {
        const auto extractedImage = new unsigned char[w * h * channels];
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
