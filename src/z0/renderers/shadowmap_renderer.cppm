module;
#include <volk.h>
#include "z0/libraries.h"

export module z0:ShadowMapRenderer;

import :Renderer;
import :Renderpass;
import :Device;
import :ShadowMapFrameBuffer;
import :Descriptors;
import :MeshInstance;
import :Light;
import :Buffer;
import :Mesh;

export namespace z0
{
    class Camera;
    /**
     * Shadow map renderer, one per light
     */
    class ShadowMapRenderer : public Renderpass, public Renderer
    {
    public:
        ShadowMapRenderer(Device &device, const string &shaderDirectory, const Light *light);

        void loadScene(const list<MeshInstance *> &meshes);

        inline void activateCamera(Camera *camera) { currentCamera = camera; }

        [[nodiscard]] mat4 getLightSpace() const;

        [[nodiscard]] inline const Light *getLight() const { return light; }

        [[nodiscard]] inline vec3 getLightPosition() const { return light->getPositionGlobal(); }

        void cleanup() override;

        [[nodiscard]] inline const shared_ptr<ShadowMapFrameBuffer> &getShadowMap() const { return shadowMap; }

        ~ShadowMapRenderer() override;

    private:
        struct GobalUniformBuffer
        {
            mat4 lightSpace;
        };

        struct ModelUniformBuffer
        {
            mat4 matrix;
        };

        // The light we render the shadow map for
        const Light *light;
        // The light is a DirectionalLight
        bool lightIsDirectional;
        // Depth bias (and slope) are used to avoid shadowing artifacts
        // Constant depth bias factor (always applied)
        const float depthBiasConstant = 1.25f;
        // Slope depth bias factor, applied depending on polygon's slope
        const float depthBiasSlope = 1.75f;
        // Scene current camera
        Camera *currentCamera{nullptr};
        // All the models of the scene
        list<MeshInstance *> models{};
        // Datas for all the models of the scene, one buffer for all the models
        // https://docs.vulkan.org/samples/latest/samples/performance/descriptor_management/README.html
        vector<unique_ptr<Buffer>> modelUniformBuffers{MAX_FRAMES_IN_FLIGHT};
        // Size of the model uniform buffer
        static constexpr VkDeviceSize modelUniformBufferSize{sizeof(ModelUniformBuffer)};
        // Currently allocated model uniform buffer count
        uint32_t modelUniformBufferCount{0};
        // The destination frame buffer
        shared_ptr<ShadowMapFrameBuffer> shadowMap;

        void update(uint32_t currentFrame) override;

        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

        void createDescriptorSetLayout() override;

        void createOrUpdateDescriptorSet(bool create) override;

        void loadShaders() override;

        void createImagesResources() override;

        void cleanupImagesResources() override;

        void recreateImagesResources() override;

        void beginRendering(VkCommandBuffer commandBuffer) override;

        void endRendering(VkCommandBuffer commandBuffer, bool isLast) override;

    public:
        ShadowMapRenderer(const ShadowMapRenderer &) = delete;

        ShadowMapRenderer &operator=(const ShadowMapRenderer &) = delete;

        ShadowMapRenderer(const ShadowMapRenderer &&) = delete;

        ShadowMapRenderer &&operator=(const ShadowMapRenderer &&) = delete;
    };

} // namespace z0
