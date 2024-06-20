#pragma once

namespace z0 {

    /**
     * Base class for offscreen frame buffer with a VKSampler attached
     */
    class SampledFrameBuffer: public BaseFrameBuffer {
    public:
        VkDescriptorImageInfo imageInfo();

    protected:
        VkSampler sampler{VK_NULL_HANDLE};

        explicit SampledFrameBuffer(const Device &dev): BaseFrameBuffer{dev} {};
    };

}