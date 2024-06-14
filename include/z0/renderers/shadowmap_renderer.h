#pragma once

namespace z0 {

    class ShadowMapRenderer: public BaseRenderpass, public BaseRenderer {
    public:
        struct GobalUniformBuffer {
            mat4 lightSpace;
        };
        struct ModelUniformBuffer {
            mat4 matrix;
        };

        ShadowMapRenderer(const Device& device, const string& shaderDirectory);

        void loadScene(shared_ptr<ShadowMapFrameBuffer>& shadowMap, list<MeshInstance*>& meshes);
        void cleanup() override;

        // Depth bias (and slope) are used to avoid shadowing artifacts
        // Constant depth bias factor (always applied)
        const float depthBiasConstant = 1.25f;
        // Slope depth bias factor, applied depending on polygon's slope
        const float depthBiasSlope = 1.75f;

    private:
        // All the models of the scene
        list<MeshInstance*> models {};
        // Datas for all the models of the scene, one buffer for all the models
        // https://docs.vulkan.org/samples/latest/samples/performance/descriptor_management/README.html
        vector<unique_ptr<Buffer>> modelUniformBuffers{MAX_FRAMES_IN_FLIGHT};
        // Size of the model uniform buffer
        static constexpr VkDeviceSize modelUniformBufferSize { sizeof(ModelUniformBuffer) };
        // Currently allocated model uniform buffer count
        uint32_t modelUniformBufferCount {0};
        shared_ptr<ShadowMapFrameBuffer> shadowMap;

        void update(uint32_t currentFrame) override;
        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;
        void createDescriptorSetLayout() override;
        void createOrUpdateDescriptorSet(bool create) override;
        void loadShaders() override;
        void createImagesResources() override;
        void cleanupImagesResources() override;
        void recreateImagesResources() override;
        void beginRendering(VkCommandBuffer commandBufferw) override;
        void endRendering(VkCommandBuffer commandBuffer, bool isLast) override;

    public:
        ShadowMapRenderer(const ShadowMapRenderer&) = delete;
        ShadowMapRenderer &operator=(const ShadowMapRenderer&) = delete;
        ShadowMapRenderer(const ShadowMapRenderer&&) = delete;
        ShadowMapRenderer &&operator=(const ShadowMapRenderer&&) = delete;
    };

}