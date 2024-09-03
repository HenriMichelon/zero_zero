module;
#include <volk.h>

export module Z0:ColorFrameBufferHDR;

import :Tools;
import :Device;
import :SampledFrameBuffer;

export namespace z0 {

    /**
     * Resolved HDR offscreen framebuffer
     */
    class ColorFrameBufferHDR: public SampledFrameBuffer {
    public:
        // HDR tone mapping
        // Table 47. Mandatory format support : 16 - bit channels
        // https://www.khronos.org/registry/vulkan/specs/1.0/pdf/vkspec.pdf
        static const VkFormat renderFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

        explicit ColorFrameBufferHDR(const Device &dev) : SampledFrameBuffer{dev } {
            ColorFrameBufferHDR::createImagesResources();
        }

        void createImagesResources() override {
            createImage(device.getSwapChainExtent().width,
                        device.getSwapChainExtent().height,
                        renderFormat,
                        VK_SAMPLE_COUNT_1_BIT, // Always resolved, only used for post-processing or display
                        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &properties);
            VkSamplerCreateInfo samplerInfo{
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

        void cleanupImagesResources() override {
            if (sampler != VK_NULL_HANDLE) {
                vkDestroySampler(device.getDevice(), sampler, nullptr);
                sampler = VK_NULL_HANDLE;
            }
            FrameBuffer::cleanupImagesResources();
        }
    };

}