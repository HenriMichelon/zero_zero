module;
#include "z0/libraries.h"
#include "vk_mem_alloc.h"

export module z0:Buffer;

import :Device;

export namespace z0 {

    /**
     * Vulkan [VkBuffer](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkBuffer.html) helper
     */
    class Buffer {
    public:
        Buffer(const Device &     dev,
               VkDeviceSize       instanceSize,
               uint32_t           instanceCount,
               VkBufferUsageFlags usageFlags,
               VkDeviceSize       minOffsetAlignment = 1);

        virtual ~Buffer();

        [[nodiscard]] inline VkBuffer getBuffer() const { return buffer; }

        [[nodiscard]] inline VkDeviceSize getAlignmentSize() const { return alignmentSize; }

        [[nodiscard]] VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size   = VK_WHOLE_SIZE,
                                                            VkDeviceSize offset = 0) const;

        [[nodiscard]] VkResult map();

        void writeToBuffer(const void * data,
                           VkDeviceSize size   = VK_WHOLE_SIZE,
                           VkDeviceSize offset = 0) const;

        void copyTo(const Buffer &dstBuffer, VkDeviceSize size) const;

    private:
        const Device &device;
        VmaAllocator  allocator;
        VkBuffer      buffer     = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        VkDeviceSize  bufferSize;
        VkDeviceSize  alignmentSize;
        void *        mapped = nullptr;

    public:
        Buffer(const Buffer &) = delete;

        Buffer &operator=(const Buffer &) = delete;
    };


}
