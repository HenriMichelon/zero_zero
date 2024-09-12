module;
#include <volk.h>

export module Z0:SampledFrameBuffer;

import :Device;
import :FrameBuffer;

export namespace z0 {

    /**
     * Base class for offscreen frame buffer with a [VKSampler](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSampler.html) attached
     */
    class SampledFrameBuffer: public FrameBuffer {
    public:
        [[nodiscard]] VkDescriptorImageInfo imageInfo() const {
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