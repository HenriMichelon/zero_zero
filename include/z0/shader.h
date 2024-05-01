#pragma once

#include "z0/device.h"

namespace z0 {

    class Shader {
    public:
        Shader(const Device& device,
               VkShaderStageFlagBits stage,
               VkShaderStageFlags nextStage,
               string name,
               const vector<char>& code,
               const VkDescriptorSetLayout* setLayouts,
               const VkPushConstantRange* pushConstantRange);
        virtual ~Shader();

        VkShaderCreateInfoEXT getShaderCreateInfo() const { return shaderCreateInfo; };
        VkShaderStageFlagBits* getStage() { return &stage; };
        VkShaderEXT* getShader() { return &shader; };

        void setShader(VkShaderEXT _shader) { shader = _shader; };

    private:
        const Device& device;
        string shaderName;
        vector<char> spirv;
        VkShaderStageFlagBits stage;
        VkShaderStageFlags stageFlags;
        VkShaderEXT shader{VK_NULL_HANDLE};
        VkShaderCreateInfoEXT shaderCreateInfo;
    };

}