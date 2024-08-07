#pragma once

namespace z0 {

    /**
     * Shadow map renderer, one per light
     */
    class ShadowMapRenderer: public Renderpass, public Renderer {
    public:
        ShadowMapRenderer(Device& device, const string& shaderDirectory, Light* light, vec3 position);
        void loadScene(list<MeshInstance*>& meshes);
        void cleanup() override;
        [[nodiscard]] const shared_ptr<ShadowMapFrameBuffer>& getShadowMap() const { return shadowMap; };
        virtual ~ShadowMapRenderer();

    private:
        // Depth bias (and slope) are used to avoid shadowing artifacts
        // Constant depth bias factor (always applied)
        const float depthBiasConstant = 1.25f;
        // Slope depth bias factor, applied depending on polygon's slope
        const float depthBiasSlope = 1.75f;

        struct GobalUniformBuffer {
            mat4 lightSpace;
        };
        struct ModelUniformBuffer {
            mat4 matrix;
        };

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