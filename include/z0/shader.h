#pragma once

namespace z0 {

    /**
     * Vulkan [VkShaderEXT](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkShaderEXT.html) helper
     */
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
        uint32_t refCount{0};
        vector<char> spirv;
        VkShaderStageFlagBits stage;
        VkShaderStageFlags stageFlags;
        VkShaderEXT shader{VK_NULL_HANDLE};
        VkShaderCreateInfoEXT shaderCreateInfo;

    public:
        void _incrementReferenceCounter();
        bool _decrementReferenceCounter();
    };

}