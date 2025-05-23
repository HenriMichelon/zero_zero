/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/vulkan.h"

export module z0.vulkan.ColorFrameBufferHDR;

import z0.vulkan.Device;
import z0.vulkan.SampledFrameBuffer;

export namespace z0 {

    /*
     * Resolved HDR offscreen framebuffer
     */
    class ColorFrameBufferHDR: public SampledFrameBuffer {
    public:
        // HDR tone mapping
        // Table 47. Mandatory format support : 16 - bit channels
        // https://www.khronos.org/registry/vulkan/specs/1.0/pdf/vkspec.pdf
        static constexpr VkFormat renderFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

        explicit ColorFrameBufferHDR(const Device &dev);
        explicit ColorFrameBufferHDR(const Device &dev, uint32_t width, uint32_t height);

        void createImagesResources() override;

        void cleanupImagesResources() override;

    private:
        uint32_t width;
        uint32_t height;
    };

}