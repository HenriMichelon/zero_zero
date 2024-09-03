/*
* https://vulkan-tutorial.com/Vertex_buffers/Vertex_buffer_creation
 * https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/
 *
 */
module;
#include <cstdlib>
#include <cstddef>
#include "z0/libraries.h"
#include <volk.h>
#include "vk_mem_alloc.h"

export module Z0:Buffer;

import :Device;
import :Tools;

export namespace z0 {

    /**
     * Vulkan [VkBuffer](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkBuffer.html) helper
     */
    class Buffer {
    public:
        Buffer(const Device &dev,
               const VkDeviceSize instanceSize,
               const uint32_t instanceCount,
               const VkBufferUsageFlags usageFlags,
               const VkDeviceSize minOffsetAlignment = 1):
        device{dev},
        allocator{dev.getAllocator()} {
            alignmentSize = minOffsetAlignment > 0 ? (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1) : instanceSize;
            bufferSize = alignmentSize * instanceCount;
            const VkBufferCreateInfo bufferInfo{
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size = bufferSize,
                .usage = usageFlags,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            };
            const VmaAllocationCreateInfo allocInfo = {
                .flags = usageFlags & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT  ?
                         VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT :
                         VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                .usage = VMA_MEMORY_USAGE_AUTO,
            };
            if (vmaCreateBuffer(allocator,
                                &bufferInfo,
                                &allocInfo,
                                &buffer,
                                &allocation,
                                nullptr) != VK_SUCCESS) {
                die("failed to create buffer!");
            }
        }

        virtual ~Buffer() {
            if (mapped) {
                vmaUnmapMemory(allocator, allocation);
                mapped = nullptr;
            }
            vmaDestroyBuffer(allocator, buffer, allocation);
        }

        [[nodiscard]] VkBuffer getBuffer() const { return buffer; }

        [[nodiscard]] VkDeviceSize getAlignmentSize() const { return alignmentSize; }

        [[nodiscard]] VkDescriptorBufferInfo descriptorInfo(const VkDeviceSize size = VK_WHOLE_SIZE, const VkDeviceSize offset = 0) const {
            return VkDescriptorBufferInfo{
                buffer,
                offset,
                size,
            };
        }

        [[nodiscard]] VkResult map() {
            return vmaMapMemory(allocator, allocation, &mapped);
        }

        void writeToBuffer(const void* data, const VkDeviceSize size = VK_WHOLE_SIZE, const VkDeviceSize offset = 0) const {
            if (size == VK_WHOLE_SIZE) {
                vmaCopyMemoryToAllocation(allocator,
                                          data,
                                          allocation,
                                          0,
                                          bufferSize);
            } else {
                vmaCopyMemoryToAllocation(allocator,
                                          data,
                                          allocation,
                                          offset,
                                          size);
            }
        }

        void copyTo(Buffer& dstBuffer, const VkDeviceSize size) const {
            VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();
            const VkBufferCopy copyRegion {
                .size = size
            };
            vkCmdCopyBuffer(commandBuffer, buffer, dstBuffer.buffer, 1, &copyRegion);
            device.endSingleTimeCommands(commandBuffer);
        }

    private:
        const Device& device;
        VmaAllocator allocator;
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
