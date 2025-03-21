/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/vulkan.h"
#include "z0/libraries.h"

export module z0.vulkan.DepthFrameBuffer;

import z0.vulkan.Device;
import z0.vulkan.SampledFrameBuffer;

export namespace z0 {

    /*
     * Depth rendering attachment or resolved offscreen depth buffer
     */
    class DepthFrameBuffer : public SampledFrameBuffer {
    public:
        explicit DepthFrameBuffer(const Device &dev, bool isMultisampled);

        void createImagesResources() override;

        void cleanupImagesResources() override;

        [[nodiscard]] inline VkDescriptorImageInfo imageInfo() const override {
            return VkDescriptorImageInfo {
                .sampler = sampler,
                .imageView = imageView,
                .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            };
        }

    private:
        bool multisampled;

        const vector<VkFormat> DEPTH_BUFFER_FORMATS[4] = {
            // DepthBufferFormat::AUTO
            {
                VK_FORMAT_X8_D24_UNORM_PACK32,
                VK_FORMAT_D32_SFLOAT,
                VK_FORMAT_D32_SFLOAT_S8_UINT,
                VK_FORMAT_D16_UNORM,
                VK_FORMAT_D16_UNORM_S8_UINT,
                VK_FORMAT_D24_UNORM_S8_UINT,
            },
            // DepthBufferFormat::B16
            {
                VK_FORMAT_D16_UNORM,
                VK_FORMAT_D16_UNORM_S8_UINT,
            },
            // DepthBufferFormat::B24
            {
                VK_FORMAT_X8_D24_UNORM_PACK32,
                VK_FORMAT_D24_UNORM_S8_UINT,
            },
            // DepthBufferFormat::B32
            {
                VK_FORMAT_D32_SFLOAT,
                VK_FORMAT_D32_SFLOAT_S8_UINT,
            },

        };
    };

}
