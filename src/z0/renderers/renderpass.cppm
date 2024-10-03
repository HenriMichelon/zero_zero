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

    /**
     * Base class for renderers and sub passes
     */
    class Renderpass {
    public:
        virtual ~Renderpass() = default;

        virtual void cleanup();

        [[nodiscard]] inline const Device &getDevice() const { return device; }

    protected:
        Device                         &device;
        VkDevice                        vkDevice;
        string                          shaderDirectory; // SPIR-V Compiled shaders directory
        VkPipelineLayout                pipelineLayout{VK_NULL_HANDLE};
        shared_ptr<DescriptorPool>      descriptorPool{};
        unique_ptr<DescriptorSetLayout> setLayout{};
        vector<VkDescriptorSet>         descriptorSet{MAX_FRAMES_IN_FLIGHT};
        unique_ptr<Shader>              vertShader;
        unique_ptr<Shader>              fragShader;
        VkDeviceSize                    globalUniformBufferSize{0};
        vector<unique_ptr<Buffer>>      globalUniformBuffers{MAX_FRAMES_IN_FLIGHT};
        bool                            descriptorSetNeedUpdate{false};
        VkClearValue                    clearColor;

        const VkClearValue depthClearValue{.depthStencil = {1.0f, 0}};

        Renderpass(Device &dev, string shaderDir, vec3 clearColor);
        Renderpass(Device &dev, string shaderDir, VkClearValue clearColor);

        // Helpers function for children classes
        static void setViewport(VkCommandBuffer commandBuffer, uint32_t width, uint32_t height);

        static void writeUniformBuffer(const vector<unique_ptr<Buffer>> &buffers, uint32_t currentFrame,
                                       const void *data, uint32_t index);

        static void writeUniformBuffer(const vector<unique_ptr<Buffer>> &buffers, uint32_t currentFrame,
                                       const void *data);

        void createOrUpdateResources();

        void createUniformBuffers(vector<unique_ptr<Buffer>> &buffers, VkDeviceSize size, uint32_t count = 1) const;

        void bindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t currentFrame, uint32_t count = 0,
                                const uint32_t *offsets = nullptr) const;

        void bindShaders(VkCommandBuffer commandBuffer) const;

        unique_ptr<Shader> createShader(const string &filename, VkShaderStageFlagBits stage,
                                        VkShaderStageFlags next_stage);

        virtual void loadShaders() = 0;

        virtual void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) = 0;

        virtual void createDescriptorSetLayout() = 0;

        virtual void createOrUpdateDescriptorSet(bool create) {}

        virtual void createPipelineLayout();

    private:
        void buildShader(Shader &shader) const;

        vector<char> readFile(const string &fileName);

    public:
        Renderpass(const Renderpass &) = delete;

        Renderpass &operator=(const Renderpass &) = delete;
    };

} // namespace z0
