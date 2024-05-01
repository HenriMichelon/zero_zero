#pragma once

#include "z0/shader.h"
#include "z0/descriptors.h"
#include "z0/resources/mesh.h"
#include "z0/resources/image.h"

namespace z0 {

    class BaseRenderpass {
    public:
        virtual void cleanup();

    protected:
        const Device& device;
        VkDevice vkDevice;
        string shaderDirectory;
        VkPipelineLayout pipelineLayout { VK_NULL_HANDLE };
        shared_ptr<DescriptorPool> descriptorPool {};
        unique_ptr<DescriptorSetLayout> setLayout {};
        vector<VkDescriptorSet> descriptorSets{MAX_FRAMES_IN_FLIGHT};
        unique_ptr<Shader> vertShader;
        unique_ptr<Shader> fragShader;
        vector<unique_ptr<Buffer>> globalUniformBuffers{MAX_FRAMES_IN_FLIGHT};

        const VkClearValue clearColor {{{
                    static_cast<float>(WINDOW_CLEAR_COLOR[0]) / 256.0f,
                    static_cast<float>(WINDOW_CLEAR_COLOR[1]) / 256.0f,
                    static_cast<float>(WINDOW_CLEAR_COLOR[2]) / 256.0f,
                    1.0f}}};
        const VkClearValue depthClearValue { .depthStencil = {1.0f, 0} };

        BaseRenderpass(const Device& device, std::string shaderDirectory);

        // Helpers function for children classes
        static void setViewport(VkCommandBuffer commandBuffer, uint32_t width, uint32_t height);
        static void writeUniformBuffer(const std::vector<std::unique_ptr<Buffer>>& buffers, uint32_t currentFrame, void *data, uint32_t index = 0);

        void createResources();
        void createUniformBuffers(std::vector<std::unique_ptr<Buffer>>& buffers, VkDeviceSize size, uint32_t count = 1);
        void bindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t currentFrame, uint32_t count = 0, uint32_t *offsets = nullptr);
        void bindShaders(VkCommandBuffer commandBuffer);
        std::unique_ptr<Shader> createShader(const std::string& filename,
                                                   VkShaderStageFlagBits stage,
                                                   VkShaderStageFlags next_stage);

        virtual void loadShaders() = 0;
        virtual void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) = 0;
        virtual void createDescriptorSetLayout() = 0;

    private:
        void buildShader(Shader& shader);
        void createPipelineLayout();
        std::vector<char> readFile(const std::string& fileName);

    public:
        BaseRenderpass(const BaseRenderpass&) = delete;
        BaseRenderpass &operator=(const BaseRenderpass&) = delete;
    };

}