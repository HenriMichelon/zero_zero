module;
#include <volk.h>
#include "z0/libraries.h"

export module z0:Renderpass;

import :Constants;
import :Device;
import :Shader;
import :Buffer;
import :Descriptors;

export namespace z0 {

    /*
     * Base class for renderers and sub passes
     */
    class Renderpass {
    public:
        virtual ~Renderpass() = default;

        virtual void cleanup();

        [[nodiscard]] inline const Device &getDevice() const { return device; }

    protected:
        Device                          &device;
        VkDevice                        vkDevice;
        VkPipelineLayout                pipelineLayout{VK_NULL_HANDLE};
        shared_ptr<DescriptorPool>      descriptorPool{};
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

        void createOrUpdateResources(bool descriptorsAndPushConstants = false, const VkPushConstantRange* = nullptr);

        unique_ptr<Buffer> createUniformBuffer(VkDeviceSize size, uint32_t count = 1) const;

        void bindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t currentFrame, uint32_t count = 0,
                                const uint32_t *offsets = nullptr) const;

        void bindShaders(VkCommandBuffer commandBuffer) const;

        unique_ptr<Shader> createShader(const string &filename, VkShaderStageFlagBits stage,
                                        VkShaderStageFlags next_stage);

        virtual void loadShaders() = 0;

        virtual void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) = 0;

        virtual void createDescriptorSetLayout() {};

        virtual void createOrUpdateDescriptorSet(bool create) {}

        virtual void createPipelineLayout(const VkPushConstantRange* = nullptr);

    private:
        void buildShader(Shader &shader) const;

    public:
        Renderpass(const Renderpass &) = delete;

        Renderpass &operator=(const Renderpass &) = delete;
    };

} // namespace z0
