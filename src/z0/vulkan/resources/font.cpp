/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <stb_image.h>
#include "z0/vulkan.h"
#include "z0/libraries.h"

module z0.resources.Font;

import z0.Tools;

import z0.resources.Image;

import z0.vulkan.Device;
import z0.vulkan.Image;

namespace z0 {

    shared_ptr<Image> Font::renderToImage(const string &text) {
        if constexpr (isImageCacheEnabled()) {
            if (imageCache.contains(text)) {
                return imageCache[text];
            }
        }
        float width, height;
        auto  bitmap = renderToBitmap(text, width, height);
        auto image = make_shared<VulkanImage>(Device::get(),
                                  text,
                                  static_cast<uint32_t>(width),
                                  static_cast<uint32_t>(height),
                                  static_cast<int>(width * height) * STBI_rgb_alpha,
                                  bitmap.data(),
                                  VK_FORMAT_R8G8B8A8_SRGB,
                                  VK_IMAGE_TILING_OPTIMAL,
                                  VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                                  VK_FILTER_LINEAR,
                                  true);
        if constexpr (isImageCacheEnabled()) {
            imageCache[text] = image;
        }
        return image;
    }

}
