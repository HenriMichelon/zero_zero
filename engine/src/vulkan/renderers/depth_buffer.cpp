#include "z0/vulkan/renderers/depth_buffer.hpp"
#include "z0/log.hpp"

namespace z0 {

    DepthBuffer::DepthBuffer(VulkanDevice &dev) :
            BaseSharedImage{dev,
        dev.findImageTilingSupportedFormat(
                {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D16_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT,},
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)} {
         createImagesResources();
     }

    DepthBuffer::~DepthBuffer() {
        cleanupImagesResources();
    }

    void DepthBuffer::createImagesResources() {
        // Create depth resources
        // https://vulkan-tutorial.com/Depth_buffering#page_Depth-image-and-view
        vulkanDevice.createImage(vulkanDevice.getSwapChainExtent().width,
                                 vulkanDevice.getSwapChainExtent().height,
                                 1,
                                 vulkanDevice.getSamples(),
                                 format,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 image, imageMemory);
        imageView = vulkanDevice.createImageView(image, format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    }

    void DepthBuffer::cleanupImagesResources() {
        if (imageMemory != VK_NULL_HANDLE) {
            vkDestroyImageView(vulkanDevice.getDevice(), imageView, nullptr);
            vkDestroyImage(vulkanDevice.getDevice(), image, nullptr);
            vkFreeMemory(vulkanDevice.getDevice(), imageMemory, nullptr);
            imageView = VK_NULL_HANDLE;
            image = VK_NULL_HANDLE;
            imageMemory = VK_NULL_HANDLE;
        }
    }


}