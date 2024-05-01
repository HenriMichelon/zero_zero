#pragma once

#include "z0/device.h"

namespace z0 {

    // Base class for all offscreen fame buffers & rendering attachements
    class BaseFrameBuffer {
    public:
        const VkImage& getImage() const { return image; }
        const VkImageView& getImageView() const { return imageView; }

        virtual void createImagesResources() = 0;
        virtual void cleanupImagesResources();

    protected:
        const Device& device;
        VkImage image;
        VkImageView imageView;
        VkDeviceMemory imageMemory;

        explicit BaseFrameBuffer(const Device &dev): device{dev} {};

        // Helper function for children classes
        void createImage(uint32_t width,
                         uint32_t height,
                         VkFormat format,
                         VkSampleCountFlagBits samples,
                         VkImageUsageFlags usage,
                         VkImageAspectFlags flags = VK_IMAGE_ASPECT_COLOR_BIT);

    public:
        BaseFrameBuffer(const BaseFrameBuffer&) = delete;
        BaseFrameBuffer &operator=(const BaseFrameBuffer&) = delete;
    };

}