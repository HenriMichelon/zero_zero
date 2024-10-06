module;
#include "z0/libraries.h"
#include <volk.h>

module z0;

import :Tools;
import :Constants;
import :Light;
import :DirectionalLight;
import :SpotLight;
import :Device;
import :ShadowMapFrameBuffer;

namespace z0 {

    ShadowMapFrameBuffer::ShadowMapFrameBuffer(const Device &dev, bool isCascaded) :
        SampledFrameBuffer{dev},
        size{static_cast<uint32_t>(isCascaded ? 4096*3 : 4096)} {
        ShadowMapFrameBuffer::createImagesResources();
    }

    void ShadowMapFrameBuffer::createImagesResources() {
        // https://github.com/SaschaWillems/Vulkan/blob/master/examples/shadowmapping/shadowmapping.cpp#L192
        // For shadow mapping we only need a depth attachment
        const auto format = device.findImageTilingSupportedFormat(
                {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D16_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT,},
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        createImage(size,
                    size,
                    format,
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                    VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_IMAGE_ASPECT_DEPTH_BIT);
        // Create sampler to sample from to depth attachment
        // Used to sample in the fragment shader for shadowed rendering
        const VkFilter shadowmap_filter = device.formatIsFilterable(format, VK_IMAGE_TILING_OPTIMAL)
                ? VK_FILTER_LINEAR
                : VK_FILTER_NEAREST;
        const VkSamplerCreateInfo samplerCreateInfo{
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .magFilter = shadowmap_filter,
                .minFilter = shadowmap_filter,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .mipLodBias = 0.0f,
                .maxAnisotropy = 1.0f,
                .minLod = 0.0f,
                .maxLod = 1.0f,
                .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE
        };
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

}
