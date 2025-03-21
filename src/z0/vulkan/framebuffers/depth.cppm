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

    private:
        bool multisampled;

        const vector<VkFormat> DEPTH_BUFFER_FORMATS[4] = {
            // DepthFormat::AUTO
            {
                VK_FORMAT_X8_D24_UNORM_PACK32,
                VK_FORMAT_D32_SFLOAT,
                VK_FORMAT_D32_SFLOAT_S8_UINT,
                VK_FORMAT_D16_UNORM,
                VK_FORMAT_D16_UNORM_S8_UINT,
                VK_FORMAT_D24_UNORM_S8_UINT,
            },
            // DepthFormat::16BIT
            {
                VK_FORMAT_D16_UNORM,
                VK_FORMAT_D16_UNORM_S8_UINT,
            },
            // DepthFormat::24BIT
            {
                VK_FORMAT_X8_D24_UNORM_PACK32,
                VK_FORMAT_D24_UNORM_S8_UINT,
            },
            // DepthFormat::32BIT
            {
                VK_FORMAT_D32_SFLOAT,
                VK_FORMAT_D32_SFLOAT_S8_UINT,
            },

        };
    };

}
