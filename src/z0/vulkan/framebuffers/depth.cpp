/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/vulkan.h"

module z0.vulkan.DepthFrameBuffer;

import z0.Application;
import z0.Tools;

import z0.vulkan.Device;

namespace z0 {

    DepthFrameBuffer::DepthFrameBuffer(const Device &dev, const bool isMultisampled):
        SampledFrameBuffer{dev},
        multisampled{isMultisampled} {
        DepthFrameBuffer::createImagesResources();
    }

    // https://vulkan-tutorial.com/Depth_buffering#page_Depth-image-and-view
    // https://docs.vulkan.org/guide/latest/depth.html
    void DepthFrameBuffer::createImagesResources() {
        createImage(
                device.getSwapChainExtent().width,
                device.getSwapChainExtent().height,
                device.findImageSupportedFormat(
                    DEPTH_BUFFER_FORMATS[static_cast<int>(app().getConfig().depthBufferFormat)],
                    VK_IMAGE_TILING_OPTIMAL,
                    multisampled ?
                        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT :
                        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT),
                multisampled ? device.getSamples() : VK_SAMPLE_COUNT_1_BIT,
                multisampled ?
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT :
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_IMAGE_ASPECT_DEPTH_BIT);

        if (!multisampled) {
            constexpr VkSamplerCreateInfo samplerCreateInfo{
                .sType         = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
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
    }

    void DepthFrameBuffer::cleanupImagesResources() {
        if (sampler != VK_NULL_HANDLE) {
            vkDestroySampler(device.getDevice(), sampler, nullptr);
            sampler = VK_NULL_HANDLE;
        }
        FrameBuffer::cleanupImagesResources();
    }


}
