module;
#include "z0/modules.h"

export module Z0:Shader;

import :Device;

export namespace z0 {

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

        [[nodiscard]] VkShaderCreateInfoEXT getShaderCreateInfo() const { return shaderCreateInfo; };
        [[nodiscard]] VkShaderStageFlagBits* getStage() { return &stage; };
        [[nodiscard]] VkShaderEXT* getShader() { return &shader; };

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
        [[nodiscard]] bool _decrementReferenceCounter();
    };


    Shader::Shader(const Device& dev,
                   VkShaderStageFlagBits stageFlagsBits,
                   VkShaderStageFlags nextStageFlags,
                   string _name,
                   const vector<char> &code,
                   const VkDescriptorSetLayout *pSetLayouts,
                   const VkPushConstantRange *pPushConstantRange):
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
        shaderCreateInfo.setLayoutCount         = 1;
        shaderCreateInfo.pSetLayouts            = pSetLayouts;
        shaderCreateInfo.pushConstantRangeCount = 0;
        shaderCreateInfo.pPushConstantRanges    = pPushConstantRange;
        shaderCreateInfo.pSpecializationInfo    = nullptr;
    }

    Shader::~Shader() {
        if (shader != VK_NULL_HANDLE) {
            vkDestroyShaderEXT(device.getDevice(), shader, nullptr);
            shader = VK_NULL_HANDLE;
        }
    }

    void Shader::_incrementReferenceCounter() {
        refCount += 1;
    }

    bool Shader::_decrementReferenceCounter() {
        refCount -= 1;
        return refCount == 0;
    }

}