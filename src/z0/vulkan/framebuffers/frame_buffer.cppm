/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>

export module z0:FrameBuffer;

import :Device;

export namespace z0 {

    /*
     * Base class for all offscreen frame buffers & rendering attachments
     */
    class FrameBuffer {
    public:
        FrameBuffer(FrameBuffer&) = delete;
        FrameBuffer(FrameBuffer&&) = delete;

        [[nodiscard]] inline const VkImage &getImage() const { return image; }

        [[nodiscard]] inline const VkImageView &getImageView() const { return imageView; }

        virtual void createImagesResources() = 0;

        virtual void cleanupImagesResources();

    protected:
        const Device & device;
        VkImage        image{nullptr};
        VkImageView    imageView{nullptr};
        VkDeviceMemory imageMemory{nullptr};

        explicit FrameBuffer(const Device &dev): device{dev} {}

        // Helper function for children classes
        void createImage(uint32_t              width,
                         uint32_t              height,
                         VkFormat              format,
                         VkSampleCountFlagBits samples,
                         VkImageUsageFlags     usage,
                         VkImageAspectFlags    flags = VK_IMAGE_ASPECT_COLOR_BIT,
                         VkImageViewType       type = VK_IMAGE_VIEW_TYPE_2D,
                         uint32_t              layers = 1,
                         VkImageCreateFlags    imageFlags = 0);

    public:
        inline virtual ~FrameBuffer() = default;

        FrameBuffer(const FrameBuffer &) = delete;

        FrameBuffer &operator=(const FrameBuffer &) = delete;
    };

}
