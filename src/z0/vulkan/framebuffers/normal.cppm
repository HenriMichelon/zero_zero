/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/vulkan.h"
#include "z0/libraries.h"

export module z0.vulkan.NormalFrameBuffer;

import z0.vulkan.Device;
import z0.vulkan.SampledFrameBuffer;

export namespace z0 {

    /*
     * Depth rendering attachment or resolved offscreen depth buffer
     */
    class NormalFrameBuffer : public SampledFrameBuffer {
    public:
        explicit NormalFrameBuffer(const Device &dev, bool isMultisampled);

        void createImagesResources() override;

        void cleanupImagesResources() override;

    private:
        bool multisampled;

        const VkFormat NORMAL_BUFFER_FORMATS[3] = {
            VK_FORMAT_R8G8B8A8_SNORM,
            VK_FORMAT_R16G16B16A16_SFLOAT,
            VK_FORMAT_R32G32B32A32_SFLOAT,
        };
    };

}
