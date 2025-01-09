/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#define VMA_VULKAN_VERSION 1003000
#include <vk_mem_alloc.h>
#include "z0/libraries.h"

export module z0.vulkan.Buffer;

export namespace z0 {

    /*
     * Vulkan [VkBuffer](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkBuffer.html) helper
     */
    class Buffer {
    public:
        Buffer(VkDeviceSize       instanceSize,
               uint32_t           instanceCount,
               VkBufferUsageFlags usageFlags,
               VkDeviceSize       minOffsetAlignment = 1);

        Buffer(Buffer &&) = delete;
        Buffer(Buffer &) = delete;

        virtual ~Buffer();

        [[nodiscard]] inline VkBuffer getBuffer() const { return buffer; }

        [[nodiscard]] inline VkDeviceSize getAlignmentSize() const { return alignmentSize; }

        [[nodiscard]] VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size   = VK_WHOLE_SIZE,
                                                            VkDeviceSize offset = 0) const;

        [[nodiscard]] VkResult map();

        void writeToBuffer(const void * data,
                           VkDeviceSize size   = VK_WHOLE_SIZE,
                           VkDeviceSize offset = 0) const;

        void copyTo(VkCommandBuffer commandBuffer, const Buffer &dstBuffer, VkDeviceSize size) const;

    private:
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
