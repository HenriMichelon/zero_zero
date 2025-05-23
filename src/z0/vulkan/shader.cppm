/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"
#include "z0/vulkan.h"

export module z0.vulkan.Shader;

import z0.vulkan.Device;

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

        [[nodiscard]] inline auto getShaderCreateInfo() const { return shaderCreateInfo; }

        [[nodiscard]] inline auto getStage() { return &stage; }

        [[nodiscard]] inline auto getShader() { return &shader; }

        inline auto setShader(const VkShaderEXT _shader) { shader = _shader; }

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
        inline auto _incrementReferenceCounter() { refCount += 1; }

        [[nodiscard]] inline auto _decrementReferenceCounter() {
            refCount -= 1;
            return refCount == 0;
        }
    };


}
