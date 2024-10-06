module;
#include <volk.h>

module z0;

import :Device;
import :FrameBuffer;

namespace z0 {

    void FrameBuffer::cleanupImagesResources() {
        if (imageMemory != VK_NULL_HANDLE) {
            vkDestroyImageView(device.getDevice(), imageView, nullptr);
            vkDestroyImage(device.getDevice(), image, nullptr);
            vkFreeMemory(device.getDevice(), imageMemory, nullptr);
            imageView   = VK_NULL_HANDLE;
            image       = VK_NULL_HANDLE;
            imageMemory = VK_NULL_HANDLE;
        }
    }

    void FrameBuffer::createImage(const uint32_t              width,
                                  const uint32_t              height,
                                  const VkFormat              format,
                                  const VkSampleCountFlagBits samples,
                                  const VkImageUsageFlags     usage,
                                  const VkImageAspectFlags    flags,
                                  const VkImageViewType       type,
                                  const uint32_t              layers) {
        device.createImage(width,
                           height,
                           1,
                           samples,
                           format,
                           VK_IMAGE_TILING_OPTIMAL,
                           usage,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                           image,
                           imageMemory,
                           0,
                           layers);
        imageView = device.createImageView(image, format, flags, 1, type, 0, layers);
    }

}
