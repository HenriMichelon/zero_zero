/*
 * Copyright (c) 2024-2025 Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <fastgltf/core.hpp>
#include "z0/vulkan.h"
#include "z0/libraries.h"

export module image;

import miplevel;
import converter;

// Image conversion destination format, one per image
export struct ImageFormat {
    string name{"bc7"};
    VkFormat format{VK_FORMAT_BC7_UNORM_BLOCK};
};

// Vulkan equivalence of compression formats with sRGB options
struct Format {
    VkFormat format;
    VkFormat formatSRGB;
};

export const map<string, Format> formats = {
    { "bc1",  {VK_FORMAT_BC1_RGBA_UNORM_BLOCK, VK_FORMAT_BC1_RGBA_SRGB_BLOCK}},
    { "bc2",  { VK_FORMAT_BC2_UNORM_BLOCK, VK_FORMAT_BC2_SRGB_BLOCK}},
    { "bc3",  { VK_FORMAT_BC3_SRGB_BLOCK, VK_FORMAT_BC3_SRGB_BLOCK}},
    { "bc4",  { VK_FORMAT_BC4_UNORM_BLOCK, VK_FORMAT_BC4_UNORM_BLOCK}},
    { "bc4s", { VK_FORMAT_BC4_SNORM_BLOCK, VK_FORMAT_BC4_SNORM_BLOCK}},
    { "bc5",  { VK_FORMAT_BC5_UNORM_BLOCK, VK_FORMAT_BC5_UNORM_BLOCK}},
    { "bc5s", { VK_FORMAT_BC5_SNORM_BLOCK, VK_FORMAT_BC5_SNORM_BLOCK}},
    { "bc7",  { VK_FORMAT_BC7_UNORM_BLOCK, VK_FORMAT_BC7_SRGB_BLOCK}},
};


// Fill the load & convert jobs queue with images from the glTF file
export void equeueImageLoading(fastgltf::Asset&   asset,
                               fastgltf::Image&   image,
                               vector<MipLevel>&  outMipLevels,
                               const string&      dstFormat,
                               bool               verbose);

// Start maxThreads threads to load & convert the images in multi-threaded batch mode
export void startImagesLoading(int maxThread);

// export uint32_t calculatePadding(const uint64_t size, const uint32_t format);
// export void saveImageDDS(const string& filename, const vector<MipLevel>& outMipLevels, const string& dstFormat);
