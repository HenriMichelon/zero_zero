module;
#include <volk.h>

module z0;

import :Tools;
import :Device;
import :ComputePipeline;

namespace z0 {

    ComputePipeline::ComputePipeline(Device &device) : Pipeline{device} {}

    VkPipeline ComputePipeline::createPipeline(const VkShaderModule shader,
                                               const VkSpecializationInfo* specializationInfo) const {
        const auto shaderStage = VkPipelineShaderStageCreateInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VK_SHADER_STAGE_COMPUTE_BIT,
            .module = shader,
            .pName = "main",
            .pSpecializationInfo = specializationInfo,
        };
        const auto createInfo = VkComputePipelineCreateInfo {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .stage = shaderStage,
            .layout = pipelineLayout,
        };
        auto pipeline = VkPipeline{VK_NULL_HANDLE};
        if(vkCreateComputePipelines(device.getDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline) != VK_SUCCESS)
            die("Failed to create compute pipeline");
        vkDestroyShaderModule(device.getDevice(), shader, nullptr);
        return pipeline;
    }

} // namespace z0
