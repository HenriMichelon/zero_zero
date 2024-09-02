module;
#include "z0/modules.h"

export module Z0:FrameBuffer;

import :Device;

export namespace z0 {

    /**
     * Base class for all offscreen frame buffers & rendering attachements
     */ 
    class FrameBuffer {
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

        explicit FrameBuffer(const Device &dev): device{dev} {};

        // Helper function for children classes
        void createImage(uint32_t width,
                         uint32_t height,
                         VkFormat format,
                         VkSampleCountFlagBits samples,
                         VkImageUsageFlags usage,
                         VkImageAspectFlags flags = VK_IMAGE_ASPECT_COLOR_BIT);

    public:
        FrameBuffer(const FrameBuffer&) = delete;
        FrameBuffer &operator=(const FrameBuffer&) = delete;
    };

    void FrameBuffer::cleanupImagesResources() {
        if (imageMemory != VK_NULL_HANDLE) {
            vkDestroyImageView(device.getDevice(), imageView, nullptr);
            vkDestroyImage(device.getDevice(), image, nullptr);
            vkFreeMemory(device.getDevice(), imageMemory, nullptr);
            imageView = VK_NULL_HANDLE;
            image = VK_NULL_HANDLE;
            imageMemory = VK_NULL_HANDLE;
        }
    }

    void FrameBuffer::createImage(uint32_t width,
                                      uint32_t height,
                                      VkFormat format,
                                      VkSampleCountFlagBits samples,
                                      VkImageUsageFlags usage,
                                      VkImageAspectFlags flags) {
        device.createImage(width, height,
                           1,samples,
                           format,VK_IMAGE_TILING_OPTIMAL,
                           usage,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                           image, imageMemory);
        imageView = device.createImageView(image, format,flags,1);
    }

}