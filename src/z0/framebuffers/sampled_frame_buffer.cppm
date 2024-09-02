module;
#include "z0/modules.h"

export module Z0:SampledFrameBuffer;

import :Device;
import :FrameBuffer;

export namespace z0 {

    /**
     * Base class for offscreen frame buffer with a VKSampler attached
     */
    class SampledFrameBuffer: public FrameBuffer {
    public:
        VkDescriptorImageInfo imageInfo() {
            return VkDescriptorImageInfo {
                .sampler = sampler,
                .imageView = imageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
        }

    protected:
        VkSampler sampler{VK_NULL_HANDLE};

        explicit SampledFrameBuffer(const Device &dev): FrameBuffer{dev} {};
    };

}