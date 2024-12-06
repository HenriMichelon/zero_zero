/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"
#include <volk.h>

module z0.vulkan.Shader;

import z0.vulkan.Device;

namespace z0 {

    Shader::Shader(const Device &               dev,
                   const VkShaderStageFlagBits  stageFlagsBits,
                   const VkShaderStageFlags     nextStageFlags,
                   string                       _name,
                   const vector<char> &         code,
                   const VkDescriptorSetLayout *pSetLayouts,
                   const VkPushConstantRange *  pPushConstantRange):
        device{dev},
        stage{stageFlagsBits},
        stageFlags{nextStageFlags},
        shaderName{std::move(_name)},
        spirv{code} {
        shaderCreateInfo.sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT;
        shaderCreateInfo.pNext                  = nullptr;
        shaderCreateInfo.flags                  = 0;
        shaderCreateInfo.stage                  = stage;
        shaderCreateInfo.nextStage              = stageFlags;
        shaderCreateInfo.codeType               = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        shaderCreateInfo.codeSize               = spirv.size() * sizeof(spirv[0]);
        shaderCreateInfo.pCode                  = spirv.data();
        shaderCreateInfo.pName                  = "main";
        shaderCreateInfo.setLayoutCount         = pSetLayouts != nullptr ? 1 : 0;
        shaderCreateInfo.pSetLayouts            = pSetLayouts;
        shaderCreateInfo.pushConstantRangeCount = pPushConstantRange != nullptr ? 1 : 0;
        shaderCreateInfo.pPushConstantRanges    = pPushConstantRange;
        shaderCreateInfo.pSpecializationInfo    = nullptr;
    }

    Shader::~Shader() {
        if (shader != VK_NULL_HANDLE) {
            vkDestroyShaderEXT(device.getDevice(), shader, nullptr);
            shader = VK_NULL_HANDLE;
        }
    }

}
