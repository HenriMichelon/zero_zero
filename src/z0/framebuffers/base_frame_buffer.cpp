#include "z0/z0.h"

namespace z0 {

    void BaseFrameBuffer::cleanupImagesResources() {
        if (imageMemory != VK_NULL_HANDLE) {
            vkDestroyImageView(device.getDevice(), imageView, nullptr);
            vkDestroyImage(device.getDevice(), image, nullptr);
            vkFreeMemory(device.getDevice(), imageMemory, nullptr);
            imageView = VK_NULL_HANDLE;
            image = VK_NULL_HANDLE;
            imageMemory = VK_NULL_HANDLE;
        }
    }

    void BaseFrameBuffer::createImage(uint32_t width,
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