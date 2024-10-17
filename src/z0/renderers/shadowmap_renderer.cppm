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
import :FrustumCulling;

// #define SHADOWMAP_RENDERER_DEBUG 1

export namespace z0 {
    class ColorFrameBufferHDR;

    class Camera;

    /**
     * Shadow map renderer, one per light
     */
    class ShadowMapRenderer : public Renderpass, public Renderer {
    public:
        ShadowMapRenderer(Device &device, const string &shaderDirectory, const Light *light);

        void loadScene(const list<MeshInstance *> &meshes);

        [[nodiscard]] inline mat4 getLightSpace(const uint32_t index) const { return lightSpace[index]; }

        inline void activateCamera(Camera *camera) { currentCamera = camera; }

        [[nodiscard]] inline const Light *getLight() const { return light; }

        [[nodiscard]] inline bool isCascaded() const { return lightIsDirectional; }

        [[nodiscard]] inline vec3 getLightPosition() const { return light->getPositionGlobal(); }

        [[nodiscard]] inline float getCascadeSplitDepth(const uint32_t index) const { return splitDepth[index]; }

        void cleanup() override;

        [[nodiscard]] inline const shared_ptr<ShadowMapFrameBuffer> &getShadowMap() const { return shadowMap; }

        ~ShadowMapRenderer() override;

#ifdef SHADOWMAP_RENDERER_DEBUG
        shared_ptr<ColorFrameBufferHDR> colorAttachmentHdr;
#endif

    private:
        struct PushConstants {
            mat4 lightSpace;
            mat4 matrix;
        };
        const VkPushConstantRange pushConstantRange {
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .offset = 0,
            .size = sizeof(PushConstants) // LightSpace VP matrix  + Model matrix
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
        Camera* currentCamera{nullptr};
        // Frustum of the camera or the spotlight
        unique_ptr<Frustum> frustum;
        // All the models of the scene
        list<MeshInstance *> models{};
        // The destination frame buffer
        shared_ptr<ShadowMapFrameBuffer> shadowMap;
        // Last computed light spaces for each cascade
        mat4 lightSpace[ShadowMapFrameBuffer::CASCADED_SHADOWMAP_LAYERS];
        // For cascaded shadow map, the last computed cascade split depth for each cascade
        float splitDepth[ShadowMapFrameBuffer::CASCADED_SHADOWMAP_LAYERS];
        // Lambda constant for split depth calculation :
        // the closer to 1.0 the smaller the firsts splits
        static constexpr auto cascadeSplitLambda = 0.75f;

        void update(uint32_t currentFrame) override;

        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

        void loadShaders() override;

        void createImagesResources() override;

        void cleanupImagesResources() override;

        void recreateImagesResources() override {};

    public:
        ShadowMapRenderer(const ShadowMapRenderer &) = delete;

        ShadowMapRenderer &operator=(const ShadowMapRenderer &) = delete;

        ShadowMapRenderer(const ShadowMapRenderer &&) = delete;

        ShadowMapRenderer &&operator=(const ShadowMapRenderer &&) = delete;
    };

} // namespace z0
