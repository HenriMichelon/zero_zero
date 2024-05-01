/*
 * https://vulkan-tutorial.com/Vertex_buffers/Vertex_buffer_creation
 * https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/
 *
 */
#include "z0/tools.h"
#include "z0/buffer.h"
#include "z0/stats.h"

namespace z0 {

    Buffer::Buffer(const Device &dev,
                   VkDeviceSize instanceSize,
                   uint32_t instanceCount,
                   VkBufferUsageFlags usageFlags,
                   VkDeviceSize minOffsetAlignment):
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
#ifdef VULKAN_STATS
        VulkanStats::get().buffersCount += 1;
#endif
    }

    Buffer::~Buffer() {
        if (mapped) {
            vmaUnmapMemory(allocator, allocation);
            mapped = nullptr;
        }
        vmaDestroyBuffer(allocator, buffer, allocation);
    }

    VkResult Buffer::map() {
        return vmaMapMemory(allocator, allocation, &mapped);
    }

    void Buffer::writeToBuffer(const void *data, const VkDeviceSize size, const VkDeviceSize offset) const {
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

    VkDescriptorBufferInfo Buffer::descriptorInfo(const VkDeviceSize size, const VkDeviceSize offset) const {
        return VkDescriptorBufferInfo{
                buffer,
                offset,
                size,
        };
    }

    void Buffer::copyTo(Buffer& dstBuffer, const VkDeviceSize size) const {
        VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();
        const VkBufferCopy copyRegion {
            .size = size
        };
        vkCmdCopyBuffer(commandBuffer, buffer, dstBuffer.buffer, 1, &copyRegion);
        device.endSingleTimeCommands(commandBuffer);
    }


}
