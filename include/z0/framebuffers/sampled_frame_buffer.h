#pragma once

namespace z0 {

    class SampledFrameBuffer: public BaseFrameBuffer {
    public:
        VkDescriptorImageInfo imageInfo();

    protected:
        VkSampler sampler{VK_NULL_HANDLE};

        explicit SampledFrameBuffer(const Device &dev): BaseFrameBuffer{dev} {};
    };

}