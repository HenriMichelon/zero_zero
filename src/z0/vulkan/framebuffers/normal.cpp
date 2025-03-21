/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/vulkan.h"

module z0.vulkan.NormalFrameBuffer;

import z0.Application;
import z0.Tools;

import z0.vulkan.Device;

namespace z0 {

    NormalFrameBuffer::NormalFrameBuffer(const Device &dev, const bool isMultisampled):
        SampledFrameBuffer{dev},
        multisampled{isMultisampled} {
        NormalFrameBuffer::createImagesResources();
    }

    void NormalFrameBuffer::createImagesResources() {
        createImage(
                device.getSwapChainExtent().width,
                device.getSwapChainExtent().height,
                NORMAL_BUFFER_FORMATS[static_cast<int>(app().getConfig().normalBufferFormat)],
                multisampled ? device.getSamples() : VK_SAMPLE_COUNT_1_BIT,
                VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        constexpr VkSamplerCreateInfo samplerCreateInfo{.sType   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                                                  .magFilter     = VK_FILTER_LINEAR,
                                                  .minFilter     = VK_FILTER_LINEAR,
                                                  .mipmapMode    = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                                                  .addressModeU  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                                  .addressModeV  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                                  .addressModeW  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                                  .mipLodBias    = 0.0f,
                                                  .maxAnisotropy = 1.0f,
                                                  .minLod        = 0.0f,
                                                  .maxLod        = 1.0f,
                                                  .borderColor   = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE};
        if (vkCreateSampler(device.getDevice(), &samplerCreateInfo, nullptr, &sampler) != VK_SUCCESS) {
            die("failed to create depth buffer sampler!");
        }
    }

    void NormalFrameBuffer::cleanupImagesResources() {
        if (sampler != VK_NULL_HANDLE) {
            vkDestroySampler(device.getDevice(), sampler, nullptr);
            sampler = VK_NULL_HANDLE;
        }
        FrameBuffer::cleanupImagesResources();
    }


}
