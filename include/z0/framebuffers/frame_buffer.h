#pragma once

namespace z0 {

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

}