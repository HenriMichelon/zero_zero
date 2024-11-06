/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>
#include "z0/libraries.h"

module z0.Pipeline;

import z0.Tools;
import z0.Device;

namespace z0 {

    Pipeline::Pipeline(Device &device) :
        device{device} {
    }

    Pipeline::~Pipeline() {
        vkDestroyPipelineLayout(device.getDevice(), pipelineLayout, nullptr);
    }

    VkImageMemoryBarrier Pipeline::imageMemoryBarrier(
            const VkImage image,
            const VkAccessFlags srcAccessMask, const VkAccessFlags dstAccessMask,
            const VkImageLayout oldLayout, const VkImageLayout newLayout,
            const uint32_t baseMipLevel,
            const uint32_t levelCount
        ) const {
        return VkImageMemoryBarrier {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask =  srcAccessMask,
            .dstAccessMask = dstAccessMask,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = baseMipLevel,
                .levelCount = levelCount,
                .baseArrayLayer = 0,
                .layerCount = VK_REMAINING_ARRAY_LAYERS,
            }
        };
    }

    void Pipeline::pipelineBarrier(
        const VkCommandBuffer commandBuffer,
        const VkPipelineStageFlags srcStageMask,
        const VkPipelineStageFlags dstStageMask,
        const vector<VkImageMemoryBarrier>& barriers) const {
        vkCmdPipelineBarrier(commandBuffer,
            srcStageMask,
            dstStageMask,
            0,
            0,
            nullptr,
            0,
            nullptr,
            static_cast<uint32_t>(barriers.size()),
            barriers.data());
    }

    VkPipelineLayout Pipeline::createPipelineLayout(const VkDescriptorSetLayout descriptorSetLayout,
                                                    const VkPushConstantRange  * pushConstants) const {
        const auto pipelineSetLayouts = array{ descriptorSetLayout };
        auto createInfo = VkPipelineLayoutCreateInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = pipelineSetLayouts.size(),
            .pSetLayouts = pipelineSetLayouts.data(),
        };
        if (pushConstants != nullptr) {
            createInfo.pushConstantRangeCount = 1;
            createInfo.pPushConstantRanges = pushConstants;
        }
        auto computePipelineLayout = VkPipelineLayout{VK_NULL_HANDLE};
        if(vkCreatePipelineLayout(device.getDevice(), &createInfo, nullptr, &computePipelineLayout) != VK_SUCCESS) {
            die("Failed to create pipeline layout");
        }
        return  computePipelineLayout;
    }

    VkShaderModule Pipeline::createShaderModule(const vector<char>& code) const {
        const auto createInfo = VkShaderModuleCreateInfo {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = code.size(),
            .pCode    = reinterpret_cast<const uint32_t*>(&code[0])
        };
        auto shaderModule = VkShaderModule{VK_NULL_HANDLE};
        if(vkCreateShaderModule(device.getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
            die("Failed to create shader module");
        return shaderModule;
    }

} // namespace z0
