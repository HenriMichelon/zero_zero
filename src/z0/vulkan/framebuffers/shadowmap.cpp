/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/vulkan.h"
#include "z0/libraries.h"

module z0.vulkan.ShadowMapFrameBuffer;

import z0.Application;
import z0.Constants;
import z0.Tools;

import z0.nodes.DirectionalLight;
import z0.nodes.Light;
import z0.nodes.SpotLight;

import z0.vulkan.Device;

namespace z0 {

    ShadowMapFrameBuffer::ShadowMapFrameBuffer(const Device &dev, const bool isCascaded, const bool isCubemap) :
        SampledFrameBuffer{dev}, isCascaded{isCascaded}, isCubemap{isCubemap} {
        if (isCascaded) {
            width = height = app().getConfig().cascadedShadowMapSize;
        } else {
            width = height = app().getConfig().pointLightShadowMapSize;
        }
        ShadowMapFrameBuffer::createImagesResources();
    }

    void ShadowMapFrameBuffer::createImagesResources() {
        for (auto i = 0; i < 6; i++) {
            cascadedImageViews[i] = nullptr;
        }
        // https://github.com/SaschaWillems/Vulkan/blob/master/examples/shadowmapping/shadowmapping.cpp#L192
        // https://github.com/SaschaWillems/Vulkan/blob/master/examples/shadowmappingomni/shadowmappingomni.cpp
        const auto format = device.findImageSupportedFormat(
                {
                    VK_FORMAT_D32_SFLOAT,
                    VK_FORMAT_D16_UNORM,
                    VK_FORMAT_D32_SFLOAT_S8_UINT,
                    VK_FORMAT_D24_UNORM_S8_UINT,
                    VK_FORMAT_D16_UNORM_S8_UINT,
                },
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        createImage(width,
                    height,
                    format,
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_IMAGE_ASPECT_DEPTH_BIT,
                    isCubemap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D_ARRAY,
                    isCascaded ? CASCADED_SHADOWMAP_MAX_LAYERS : isCubemap ? 6 : 1,
                    isCubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0);
        if (isCascaded || isCubemap) {
            // Create additional image views for the cascaded shadow map renderer
            // One view for each shadow map split of the cascade
            for (auto i = 0; i < (isCascaded ? CASCADED_SHADOWMAP_MAX_LAYERS : 6); i++) {
                cascadedImageViews[i] =
                    device.createImageView(image, format, VK_IMAGE_ASPECT_DEPTH_BIT, 1, VK_IMAGE_VIEW_TYPE_2D_ARRAY, i);
            }
        }
        // Create sampler for the depth attachment.
        // Used to sample in the main fragment shader for the shadow factor calculations
        // const VkFilter shadowmapFilter =
        // device.formatIsFilterable(format, VK_IMAGE_TILING_OPTIMAL) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
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
            die("failed to create shadowmap sampler!");
        }
    }

    void ShadowMapFrameBuffer::cleanupImagesResources() {
        for (auto i = 0; i < 6; i++) {
            if (cascadedImageViews[i] != nullptr) {
                vkDestroyImageView(device.getDevice(), cascadedImageViews[i], nullptr);
                cascadedImageViews[i] = nullptr;
            }
        }
        if (sampler != VK_NULL_HANDLE) {
            vkDestroySampler(device.getDevice(), sampler, nullptr);
            sampler = VK_NULL_HANDLE;
        }
        FrameBuffer::cleanupImagesResources();
    }

} // namespace z0
