/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/vulkan.h"
#include "z0/libraries.h"

export module z0.vulkan.Pipeline;

import z0.resources.Image;

import z0.vulkan.Device;

export namespace z0 {

    /*
     * Base class for all pipelines
     */
    class Pipeline {
    public:
        Pipeline(Pipeline&) = delete;
        Pipeline(Pipeline&&) = delete;
        explicit Pipeline(Device &device);
        virtual ~Pipeline();

    protected:
        Device &device;
        VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};

        VkPipelineLayout createPipelineLayout(
            VkDescriptorSetLayout descriptorSetLayout,
                                              const VkPushConstantRange * pushConstants = nullptr) const;

        VkShaderModule createShaderModule(const vector<char>& code) const;

        VkImageMemoryBarrier imageMemoryBarrier(
            VkImage image,
            VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
            VkImageLayout oldLayout, VkImageLayout newLayout,
            uint32_t baseMipLevel = 0,
            uint32_t levelCount = VK_REMAINING_MIP_LEVELS
        ) const;

        void pipelineBarrier(
            VkCommandBuffer commandBuffer,
            VkPipelineStageFlags srcStageMask,
            VkPipelineStageFlags dstStageMask,
            const vector<VkImageMemoryBarrier>& barriers) const;
    };

}
