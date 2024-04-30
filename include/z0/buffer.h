#pragma once

#include "z0/device.h"

namespace z0 {

    class Buffer {
    public:
        Buffer(Device& device,
               VkDeviceSize instanceSize,
               uint32_t instanceCount,
               VkBufferUsageFlags usageFlags,
               VkDeviceSize minOffsetAlignment = 1);
        ~Buffer();

        VkBuffer getBuffer() const { return buffer; }
        VkDeviceSize getAlignmentSize() const { return alignmentSize; }
        VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;

        VkResult map();
        void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
        void copyTo(Buffer& dstBuffer, VkDeviceSize size) const;

    private:
        Device& device;
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        VkDeviceSize bufferSize;
        VkDeviceSize alignmentSize;
        void* mapped = nullptr;

    public:
        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;
    };

}
