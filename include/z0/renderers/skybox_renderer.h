#pragma once

namespace z0 {

    class SkyboxRenderer: public BaseRenderpass {
    public:
        SkyboxRenderer(Device& device, const string& shaderDirectory);

        void loadScene(shared_ptr<Cubemap>& cubemap);

        void cleanup() override;
        void update(Camera* currentCamera, Environment* currentEnvironment, uint32_t currentFrame);
        void loadShaders() override;
        void createDescriptorSetLayout() override;
        void createOrUpdateDescriptorSet(bool create) override;
        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

    private:
        struct GobalUniformBuffer {
            mat4 projection{1.0f};
            mat4 view{1.0f};
            vec4 ambient{1.0f, 1.0f, 1.0f, 1.0f}; // RGB + Intensity;
        };

        uint32_t vertexCount;
        shared_ptr<Cubemap> cubemap;
        unique_ptr<Buffer> vertexBuffer;
    };

}