module;
#include <stb_image.h>
#include <stb_image_write.h>
#include "z0/libraries.h"

module z0;

import :Tools;
import :Image;

namespace z0 {

    Image::Image(
        const uint32_t             width,
        const uint32_t             height,
        const string &             name):
        Resource{name}, width{width}, height{height} {};

    void vr_stb_write_func(void *context, void *data, const int size) {
        auto *buffer = static_cast<vector<unsigned char> *>(context);
        auto *ptr    = static_cast<unsigned char *>(data);
        buffer->insert(buffer->end(), ptr, ptr + size);
    }

    shared_ptr<Image> Image::createBlankImage() {
        vector<unsigned char> blankJPEG;
        const auto data = new unsigned char[1 * 1 * 3];
        data[0]   = 0;
        data[1]   = 0;
        data[2]   = 0;
        stbi_write_jpg_to_func(vr_stb_write_func, &blankJPEG, 1, 1, 3, data, 100);
        delete[] data;
        return create(1, 1, blankJPEG.size(), blankJPEG.data(), "Blank");
    }

    shared_ptr<Image> Image::loadFromFile(const string &filename) {
        const auto &filepath = (Application::get().getConfig().appDir / filename).string();
        // Create texture image
        // https://vulkan-tutorial.com/Texture_mapping/Images#page_Loading-an-image
        int   texWidth, texHeight, texChannels;
        auto          *pixels    = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        const uint64_t imageSize = texWidth * texHeight * STBI_rgb_alpha;
        if (!pixels) die("failed to load texture image!");
        auto image = create(
            static_cast<uint32_t>(texWidth),
            static_cast<uint32_t>(texHeight),
            imageSize,
            static_cast<void *>(pixels),
            filepath);
        stbi_image_free(pixels);
        return image;
    }

}
