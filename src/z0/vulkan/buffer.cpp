/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
/*
 * https://vulkan-tutorial.com/Vertex_buffers/Vertex_buffer_creation
 * https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/
 *
 */
module;
#include "z0/libraries.h"
#include "z0/vulkan.h"
#include "vk_mem_alloc.h"

module z0.vulkan.Buffer;

import z0.Log;
import z0.Tools;

import z0.vulkan.Device;

namespace z0 {

    // static list<VkBuffer> buffers;

    Buffer::Buffer(const VkDeviceSize       instanceSize,
                   const uint32_t           instanceCount,
                   const VkBufferUsageFlags usageFlags,
                   const VkDeviceSize       minOffsetAlignment):
        allocator{Device::get().getAllocator()} {
        alignmentSize = minOffsetAlignment > 0
                ? (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1)
                : instanceSize;
        bufferSize = alignmentSize * instanceCount;
        const VkBufferCreateInfo bufferInfo{
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size = bufferSize,
                .usage = usageFlags,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };
        const VmaAllocationCreateInfo allocInfo = {
                .flags = static_cast<VmaAllocationCreateFlags>(usageFlags & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
                ? VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
                : VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT),
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
        DEBUG("Created Buffer ", instanceSize , "*", instanceCount, " : ", to_hexstring(buffer));
    }

    Buffer::~Buffer() {
        DEBUG("Destroy Buffer : ", to_hexstring(buffer));
        if (mapped) {
            vmaUnmapMemory(allocator, allocation);
            mapped = nullptr;
        }
        vmaDestroyBuffer(allocator, buffer, allocation);
    }

    VkDescriptorBufferInfo Buffer::descriptorInfo(const VkDeviceSize size,
                                                  const VkDeviceSize offset) const {
        return VkDescriptorBufferInfo{
                buffer,
                offset,
                size,
        };
    }

    VkResult Buffer::map() {
        return vmaMapMemory(allocator, allocation, &mapped);
    }

    void Buffer::writeToBuffer(const void *       data,
                               const VkDeviceSize size,
                               const VkDeviceSize offset) const {
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

    void Buffer::copyTo(const VkCommandBuffer commandBuffer, const Buffer &dstBuffer, const VkDeviceSize size) const {
        const VkBufferCopy copyRegion {
            .size = size
        };
        vkCmdCopyBuffer(commandBuffer, buffer, dstBuffer.buffer, 1, &copyRegion);
    }


}
