#pragma once

namespace z0 {

    // Resolved HDR offscreen framebuffer
    class ColorFrameBufferHDR: public BaseFrameBuffer {
    public:
        // HDR tone mapping
        // Table 47. Mandatory format support : 16 - bit channels
        // https://www.khronos.org/registry/vulkan/specs/1.0/pdf/vkspec.pdf
        static const VkFormat renderFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

        explicit ColorFrameBufferHDR(const Device &dev);
        void createImagesResources() override;
        void cleanupImagesResources() override;
        VkDescriptorImageInfo imageInfo();

    private:
        VkSampler sampler{VK_NULL_HANDLE};
    };

}