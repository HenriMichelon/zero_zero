module;
#include <volk.h>
#include "z0/libraries.h"

module z0;

import :Tools;
import :Device;
import :Pipeline;

namespace z0 {

    Pipeline::Pipeline(Device &device) :
        device{device} {
    }

    Pipeline::~Pipeline() {
        vkDestroyPipelineLayout(device.getDevice(), pipelineLayout, nullptr);
    }

    VkPipeline Pipeline::createPipeline(const VkShaderModule shader) const {
        const auto shaderStage = VkPipelineShaderStageCreateInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VK_SHADER_STAGE_COMPUTE_BIT,
            .module = shader,
            .pName = "main",
            .pSpecializationInfo = nullptr,
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

    VkPipelineLayout Pipeline::createPipelineLayout(VkDescriptorSetLayout descriptorSetLayout) const {
        const auto pipelineSetLayouts = array{ descriptorSetLayout };
        const auto createInfo = VkPipelineLayoutCreateInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = pipelineSetLayouts.size(),
            .pSetLayouts = pipelineSetLayouts.data(),
        };
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

    vector<char> Pipeline::readFile(const string &fileName) const {
        filesystem::path filepath = (Application::get().getConfig().appDir / "shaders").string();
        filepath /= fileName;
        filepath += ".spv";
        ifstream file{filepath, std::ios::ate | std::ios::binary};
        if (!file.is_open()) {
            die("failed to open file : ", filepath.string());
        }
        const size_t fileSize = file.tellg();
        vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }
} // namespace z0
