/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/vulkan.h"
#include "z0/libraries.h"

export module z0.vulkan.Renderpass;

import z0.Constants;

import z0.vulkan.Buffer;
import z0.vulkan.Descriptors;
import z0.vulkan.Device;
import z0.vulkan.Shader;

export namespace z0 {

    /*
     * Base class for renderers and sub passes
     */
    class Renderpass {
    public:
        Renderpass(Renderpass&&) = delete;
        Renderpass(Renderpass&) = delete;
        virtual ~Renderpass() = default;

        virtual void cleanup();

        [[nodiscard]] inline const Device &getDevice() const { return device; }

    protected:
        Device                          &device;
        VkDevice                        vkDevice;
        VkPipelineLayout                pipelineLayout{VK_NULL_HANDLE};
        unique_ptr<DescriptorPool>      descriptorPool{};
        unique_ptr<DescriptorSetLayout> setLayout{};
        VkPushConstantRange             *pushConstantRange{nullptr};
        vector<VkDescriptorSet>         descriptorSet;
        unique_ptr<Shader>              vertShader;
        unique_ptr<Shader>              fragShader;
        bool                            descriptorSetNeedUpdate{false};
        VkClearValue                    clearColor;

        const VkClearValue depthClearValue{.depthStencil = {1.0f, 0}};

        Renderpass(Device &dev, vec3 clearColor);
        Renderpass(Device &dev, VkClearValue clearColor);

        // Helpers function for children classes
        static void setViewport(VkCommandBuffer commandBuffer, uint32_t width, uint32_t height);

        static void writeUniformBuffer(const unique_ptr<Buffer> &buffer, const void *data, uint32_t index);

        static void writeUniformBuffer(const unique_ptr<Buffer> &buffer, const void *data);

        void createOrUpdateResources(bool descriptorsAndPushConstants = false, const VkPushConstantRange* = nullptr, uint32_t pushConstantSize = 0);

        unique_ptr<Buffer> createUniformBuffer(VkDeviceSize size, uint32_t count = 1) const;

        void bindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t currentFrame, uint32_t count = 0,
                                const uint32_t *offsets = nullptr) const;

        void bindShaders(VkCommandBuffer commandBuffer) const;

        unique_ptr<Shader> createShader(const string &filename, VkShaderStageFlagBits stage,
                                        VkShaderStageFlags next_stage);

        virtual void loadShaders() = 0;

        virtual void createDescriptorSetLayout() {}

        virtual void createOrUpdateDescriptorSet(bool create) {}

        virtual void createPipelineLayout(const VkPushConstantRange* = nullptr, uint32_t pushConstantSize = 0);

    private:
        void buildShader(Shader &shader) const;

    public:
        Renderpass(const Renderpass &) = delete;

        Renderpass &operator=(const Renderpass &) = delete;
    };

} // namespace z0
