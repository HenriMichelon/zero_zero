/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/vulkan.h"

export module z0.vulkan.SampledFrameBuffer;

import z0.vulkan.Device;
import z0.vulkan.FrameBuffer;

export namespace z0 {

    /*
     * Base class for offscreen frame buffer with a [VKSampler](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSampler.html) attached
     */
    class SampledFrameBuffer: public FrameBuffer {
    public:
        [[nodiscard]] virtual inline VkDescriptorImageInfo imageInfo() const  {
            return VkDescriptorImageInfo {
                .sampler = sampler,
                .imageView = imageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
        }

    protected:
        VkSampler sampler{VK_NULL_HANDLE};

        explicit SampledFrameBuffer(const Device &dev): FrameBuffer{dev} {}
    };

}