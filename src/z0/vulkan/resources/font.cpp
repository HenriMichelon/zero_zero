/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <stb_image.h>
#include <volk.h>
#include "z0/libraries.h"

module z0.Font;

import z0.Tools;
import z0.Image;

import z0.Device;
import z0.VulkanImage;

namespace z0 {

    shared_ptr<Image> Font::renderToImage(const string &text) {
        float width, height;
        auto  bitmap = renderToBitmap(text, width, height);
        /*  auto name = str;
         name.append(".png");
         stbi_write_png(name.c_str(), width, height, STBI_rgb_alpha, bitmap.data(), width * STBI_rgb_alpha); */
        return make_shared<VulkanImage>(Device::get(),
                                  text,
                                  static_cast<uint32_t>(width),
                                  static_cast<uint32_t>(height),
                                  static_cast<int>(width * height) * STBI_rgb_alpha,
                                  bitmap.data(),
                                  VK_FORMAT_R8G8B8A8_SRGB,
                                  VK_IMAGE_TILING_OPTIMAL,
                                  VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                                  VK_FILTER_LINEAR,
                                  // don't repeat texture
                                  false);
    }

}
