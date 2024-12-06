/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <cassert>
#include <volk.h>
#include "z0/libraries.h"

module z0.resources.Cubemap;

import z0.Application;
import z0.Constants;
import z0.resources.Image;
import z0.Tools;
import z0.VirtualFS;

import z0.vulkan.Device;
import z0.vulkan.IBLPipeline;
import z0.vulkan.Cubemap;
import z0.vulkan.Image;

namespace z0 {

    shared_ptr<Cubemap> Cubemap::create(
       const uint32_t      width,
       const uint32_t      height,
       const uint32_t      imageSize,
       const vector<byte*> &data,
       const string &      name) {
        return make_shared<VulkanCubemap>(Device::get(), width, height, imageSize, data, name);
    }

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

    shared_ptr<Cubemap> Cubemap::load(const string &filepath, const string &fileext, const ImageFormat imageFormat) {
        uint32_t texWidth, texHeight;
        uint64_t imageSize;
        vector<byte *> data;
        const array<std::string, 6> names{"right", "left", "top", "bottom", "front", "back"};
        for (int i = 0; i < 6; i++) {
            auto path = filepath + "_" + names[i] + fileext;
            auto *pixels = VirtualFS::loadRGBAImage(path, texWidth, texHeight, imageSize, imageFormat);
            if (!pixels) { die("failed to load texture image", path); }
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

    shared_ptr<Cubemap> Cubemap::load(const string &filepath, const ImageFormat imageFormat) {
        assert(imageFormat == ImageFormat::R8G8B8A8_SRGB);
        uint32_t texWidth, texHeight;
        uint64_t imageSize;
        auto *pixels = VirtualFS::loadRGBAImage(filepath, texWidth, texHeight, imageSize, imageFormat);
        if (!pixels) { die("failed to load texture image", filepath); }
        vector<byte*> data;
        const auto imgWidth  = texWidth / 4;
        const auto imgHeight = texHeight / 3;
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

    EnvironmentCubemap::EnvironmentCubemap(const string &  name):
            Cubemap{ENVIRONMENT_MAP_SIZE, ENVIRONMENT_MAP_SIZE, TYPE_ENVIRONMENT, name} {
        auto& device = Device::get();
        specularCubemap = make_shared<VulkanCubemap>(device,
            ENVIRONMENT_MAP_SIZE,
            ENVIRONMENT_MAP_SIZE,
            ENVIRONMENT_MAP_MIPMAP_LEVELS);
        irradianceCubemap = make_shared<VulkanCubemap>(device,
           IRRADIANCE_MAP_SIZE,
           IRRADIANCE_MAP_SIZE);
        brdfLut = make_shared<VulkanImage>(device,
            BRDFLUT_SIZE,
            BRDFLUT_SIZE,
            1,
            VK_FORMAT_R16G16_SFLOAT,
            1,
            VK_IMAGE_USAGE_STORAGE_BIT,
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            VK_FILTER_LINEAR,
            VK_FALSE
        );
    }

    shared_ptr<EnvironmentCubemap> EnvironmentCubemap::loadFromHDRi(const string &filename, ImageFormat imageFormat) {
        auto& device = Device::get();
        auto envCubemap = make_shared<EnvironmentCubemap>();
        const auto unfilteredCubemap = make_shared<VulkanCubemap>(device,
            ENVIRONMENT_MAP_SIZE,
            ENVIRONMENT_MAP_SIZE
        );
        const auto &vkSpecular = reinterpret_pointer_cast<VulkanCubemap>(envCubemap->specularCubemap);
        const auto &vkIrradiance = reinterpret_pointer_cast<VulkanCubemap>(envCubemap->irradianceCubemap);
        const auto &vkBRDF = reinterpret_pointer_cast<VulkanImage>(envCubemap->brdfLut);
        const auto iblPipeline = IBLPipeline{device};
        iblPipeline.convert(
            reinterpret_pointer_cast<VulkanImage>(
                Image::load(filename, imageFormat)),
                unfilteredCubemap,
                vkSpecular,
                vkIrradiance,
                vkBRDF);
        return envCubemap;
    }

}
