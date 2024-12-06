/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>

module z0.vulkan.ColorFrameBufferHDR;

import z0.Tools;

import z0.vulkan.Device;

namespace z0 {

    ColorFrameBufferHDR::ColorFrameBufferHDR(const Device &dev) :
        SampledFrameBuffer{dev},
        width{device.getSwapChainExtent().width},
        height{device.getSwapChainExtent().height} {
        ColorFrameBufferHDR::createImagesResources();
    }

    ColorFrameBufferHDR::ColorFrameBufferHDR(const Device &dev, const uint32_t width, const uint32_t height) :
        SampledFrameBuffer{dev},
        width{width},
        height{height} {
        ColorFrameBufferHDR::createImagesResources();
    }

    void ColorFrameBufferHDR::createImagesResources() {
        createImage(width, height,
                    renderFormat,
                    VK_SAMPLE_COUNT_1_BIT,
                    // Always resolved, only used for post-processing or display
                    VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT);

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &properties);
        const VkSamplerCreateInfo samplerInfo{
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .magFilter = VK_FILTER_LINEAR,
                .minFilter = VK_FILTER_LINEAR,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .mipLodBias = 0.0f,
                .anisotropyEnable = VK_TRUE,
                .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
                .minLod = 0.0f,
                .maxLod = 1.0f,
                .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                .unnormalizedCoordinates = VK_FALSE,
        };
        if (vkCreateSampler(device.getDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
            die("failed to create color attachment sampler!");
        }
    }

    void ColorFrameBufferHDR::cleanupImagesResources() {
        if (sampler != VK_NULL_HANDLE) {
            vkDestroySampler(device.getDevice(), sampler, nullptr);
            sampler = VK_NULL_HANDLE;
        }
        FrameBuffer::cleanupImagesResources();
    }

}
