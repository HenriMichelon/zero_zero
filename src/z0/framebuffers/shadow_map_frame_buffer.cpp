module;
#include <volk.h>
#include "z0/libraries.h"

module z0;

import :Tools;
import :Constants;
import :Light;
import :DirectionalLight;
import :SpotLight;
import :Device;
import :ShadowMapFrameBuffer;

namespace z0 {

    ShadowMapFrameBuffer::ShadowMapFrameBuffer(const Device &dev, const bool isCascaded) :
        SampledFrameBuffer{dev}, isCascaded{isCascaded} {
        ShadowMapFrameBuffer::createImagesResources();
    }

    void ShadowMapFrameBuffer::createImagesResources() {
        // https://github.com/SaschaWillems/Vulkan/blob/master/examples/shadowmapping/shadowmapping.cpp#L192
        // For shadow mapping we only need a depth attachment
        const auto format = device.findImageTilingSupportedFormat(
                {
                        VK_FORMAT_D32_SFLOAT,
                        VK_FORMAT_D16_UNORM,
                        VK_FORMAT_D32_SFLOAT_S8_UINT,
                        VK_FORMAT_D24_UNORM_S8_UINT,
                },
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        createImage(size,
                    size,
                    format,
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                            VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_IMAGE_ASPECT_DEPTH_BIT,
                    // Use a cascaded shadow map with layered image for directional light
                    isCascaded ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D,
                    isCascaded ? CASCADED_SHADOWMAP_LAYERS : 1);
        // Create additional image views for the shadow map renderer
        if (isCascaded) {
            for (int i = 0; i < CASCADED_SHADOWMAP_LAYERS; i++) {
                cascadedImageViews[i] =
                        device.createImageView(image, format, VK_IMAGE_ASPECT_DEPTH_BIT, 1, VK_IMAGE_VIEW_TYPE_2D_ARRAY, i);
            }
        }
        // Create sampler for the depth attachment.
        // Used to sample in the main fragment shader for shadowed rendering
        const VkFilter shadowmap_filter =
                device.formatIsFilterable(format, VK_IMAGE_TILING_OPTIMAL) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
        const VkSamplerCreateInfo samplerCreateInfo{.sType         = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                                                    .magFilter     = shadowmap_filter,
                                                    .minFilter     = shadowmap_filter,
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
        if (sampler != VK_NULL_HANDLE) {
            vkDestroySampler(device.getDevice(), sampler, nullptr);
            sampler = VK_NULL_HANDLE;
        }
        FrameBuffer::cleanupImagesResources();
    }

} // namespace z0
