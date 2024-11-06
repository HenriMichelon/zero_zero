/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"
#include <volk.h>

export module z0.Shader;

import z0.Device;

export namespace z0 {

    /*
     * Vulkan [VkShaderEXT](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkShaderEXT.html) helper
     */
    class Shader {
    public:
        Shader(const Device &               dev,
               VkShaderStageFlagBits        stageFlagsBits,
               VkShaderStageFlags           nextStageFlags,
               string                       _name,
               const vector<char> &         code,
               const VkDescriptorSetLayout *pSetLayouts,
               const VkPushConstantRange *  pPushConstantRange);

        Shader(Shader &&) = delete;
        Shader(Shader &) = delete;

        virtual ~Shader();

        [[nodiscard]] inline VkShaderCreateInfoEXT getShaderCreateInfo() const { return shaderCreateInfo; }

        [[nodiscard]] inline VkShaderStageFlagBits *getStage() { return &stage; }

        [[nodiscard]] inline VkShaderEXT *getShader() { return &shader; }

        inline void setShader(const VkShaderEXT _shader) { shader = _shader; }

    private:
        const Device &        device;
        string                shaderName;
        uint32_t              refCount{0};
        vector<char>          spirv;
        VkShaderStageFlagBits stage;
        VkShaderStageFlags    stageFlags;
        VkShaderEXT           shader{VK_NULL_HANDLE};
        VkShaderCreateInfoEXT shaderCreateInfo;

    public:
        inline void _incrementReferenceCounter() { refCount += 1; }

        [[nodiscard]] inline bool _decrementReferenceCounter() {
            refCount -= 1;
            return refCount == 0;
        }
    };


}
