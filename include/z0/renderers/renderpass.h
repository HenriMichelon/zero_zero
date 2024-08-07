#pragma once

namespace z0 {

    /**
     * Base class for renderers and sub passes
     */
    class Renderpass {
    public:
        virtual void cleanup();

        [[nodiscard]] const Device& getDevice() const { return device; }

    protected:
        Device& device;
        VkDevice vkDevice;
        string shaderDirectory; // SPIR-V Compiled shaders directory
        VkPipelineLayout pipelineLayout { VK_NULL_HANDLE };
        shared_ptr<DescriptorPool> descriptorPool {};
        unique_ptr<DescriptorSetLayout> setLayout {};
        vector<VkDescriptorSet> descriptorSet{MAX_FRAMES_IN_FLIGHT};
        unique_ptr<Shader> vertShader;
        unique_ptr<Shader> fragShader;
        VkDeviceSize globalUniformBufferSize;
        vector<unique_ptr<Buffer>> globalUniformBuffers{MAX_FRAMES_IN_FLIGHT};
        bool descriptorSetNeedUpdate{false};

        const VkClearValue clearColor {{{
                    static_cast<float>(WINDOW_CLEAR_COLOR[0]) / 256.0f,
                    static_cast<float>(WINDOW_CLEAR_COLOR[1]) / 256.0f,
                    static_cast<float>(WINDOW_CLEAR_COLOR[2]) / 256.0f,
                    1.0f}}};
        const VkClearValue depthClearValue { .depthStencil = {1.0f, 0} };

        Renderpass(Device& device, const string& shaderDirectory);

        // Helpers function for children classes
        static void setViewport(VkCommandBuffer commandBuffer, uint32_t width, uint32_t height);
        static void writeUniformBuffer(const vector<unique_ptr<Buffer>>& buffers, uint32_t currentFrame, void *data, uint32_t index);
        static void writeUniformBuffer(const vector<unique_ptr<Buffer>>& buffers, uint32_t currentFrame, void *data);

        void createOrUpdateResources();
        void createUniformBuffers(vector<unique_ptr<Buffer>>& buffers, VkDeviceSize size, uint32_t count = 1);
        void bindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t currentFrame, uint32_t count = 0, uint32_t *offsets = nullptr);
        void bindShaders(VkCommandBuffer commandBuffer);
        unique_ptr<Shader> createShader(const string& filename, VkShaderStageFlagBits stage, VkShaderStageFlags next_stage);

        virtual void loadShaders() = 0;
        virtual void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) = 0;
        virtual void createDescriptorSetLayout() = 0;
        virtual void createOrUpdateDescriptorSet(bool create) { };
        virtual void createPipelineLayout();

    private:
        void buildShader(Shader& shader);
        vector<char> readFile(const string& fileName);

    public:
        Renderpass(const Renderpass&) = delete;
        Renderpass &operator=(const Renderpass&) = delete;
    };

}