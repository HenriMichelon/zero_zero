module;
#include <volk.h>

export module Z0:FrameBuffer;

import :Device;

export namespace z0 {

    /**
     * Base class for all offscreen frame buffers & rendering attachments
     */ 
    class FrameBuffer {
    public:
        [[nodiscard]] const VkImage& getImage() const { return image; }

        [[nodiscard]] const VkImageView& getImageView() const { return imageView; }

        virtual void createImagesResources() = 0;

        virtual void cleanupImagesResources() {
            if (imageMemory != VK_NULL_HANDLE) {
                vkDestroyImageView(device.getDevice(), imageView, nullptr);
                vkDestroyImage(device.getDevice(), image, nullptr);
                vkFreeMemory(device.getDevice(), imageMemory, nullptr);
                imageView = VK_NULL_HANDLE;
                image = VK_NULL_HANDLE;
                imageMemory = VK_NULL_HANDLE;
            }
        }

    protected:
        const Device& device;
        VkImage image{nullptr};
        VkImageView imageView{nullptr};
        VkDeviceMemory imageMemory{nullptr};

        explicit FrameBuffer(const Device &dev): device{dev} {};

        // Helper function for children classes
        void createImage(const uint32_t width,
                         const uint32_t height,
                         const VkFormat format,
                         const VkSampleCountFlagBits samples,
                         const VkImageUsageFlags usage,
                         const VkImageAspectFlags flags = VK_IMAGE_ASPECT_COLOR_BIT) {
            device.createImage(width, height,
                               1,samples,
                               format,VK_IMAGE_TILING_OPTIMAL,
                               usage,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                               image, imageMemory);
            imageView = device.createImageView(image, format,flags,1);
        }

    public:
        inline virtual ~FrameBuffer() = default;
        FrameBuffer(const FrameBuffer&) = delete;
        FrameBuffer &operator=(const FrameBuffer&) = delete;
    };

}