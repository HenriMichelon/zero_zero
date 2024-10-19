module;
#include <volk.h>
#include "z0/libraries.h"

export module z0:ShadowMapRenderer;

import :Constants;
import :Renderer;
import :Renderpass;
import :Device;
import :ShadowMapFrameBuffer;
import :Descriptors;
import :MeshInstance;
import :Light;
import :DirectionalLight;
import :Buffer;
import :Mesh;
import :FrustumCulling;

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

        [[nodiscard]] inline mat4 getLightSpace(const uint32_t index, const uint32_t currentFrame) const {
            return frameData[currentFrame].lightSpace[index];
        }

        inline void activateCamera(Camera *camera, const uint32_t currentFrame) {
            frameData[currentFrame].currentCamera = camera;
        }

        [[nodiscard]] inline const Light *getLight() const { return light; }

        [[nodiscard]] inline bool isCascaded() const { return directionalLight != nullptr; }

        [[nodiscard]] inline vec3 getLightPosition() const { return light->getPositionGlobal(); }

        [[nodiscard]] inline float getCascadesCount(const uint32_t currentFrame) const {
            return frameData[currentFrame].cascadesCount;
        }

        [[nodiscard]] inline float getCascadeSplitDepth(const uint32_t index, const uint32_t currentFrame) const {
            return frameData[currentFrame].splitDepth[index];
        }

        [[nodiscard]] inline const shared_ptr<ShadowMapFrameBuffer> &getShadowMap(const uint32_t currentFrame) const {
            return frameData[currentFrame].shadowMap;
        }

        void cleanup() override;

        ~ShadowMapRenderer() override;

    private:
        struct PushConstants {
            mat4 lightSpace;
            mat4 matrix;
        };
        static constexpr auto PUSHCONSTANTS_SIZE{sizeof(PushConstants)};
        static constexpr  VkPushConstantRange pushConstantRange {
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .offset = 0,
            .size = PUSHCONSTANTS_SIZE
        };

        // Depth bias (and slope) are used to avoid shadowing artifacts
        // Constant depth bias factor (always applied)
        static constexpr float depthBiasConstant = 1.25f;
        // Slope depth bias factor, applied depending on polygon's slope
        static constexpr  float depthBiasSlope = 1.75f;
        // Lambda constant for split depth calculation :
        // the closer to 1.0 the smaller the firsts splits
        static constexpr auto cascadeSplitLambda = 0.95f;

        // The light we render the shadow map for
        const Light *light;
        // The light is a DirectionalLight
        const DirectionalLight* directionalLight{nullptr};

        struct {
            // Scene current camera
            Camera* currentCamera{nullptr};
            // All the models of the scene
            list<MeshInstance *> models{};
            // The destination frame buffer
            shared_ptr<ShadowMapFrameBuffer> shadowMap;
            // Number of cascades
            uint32_t cascadesCount{1};
            // Last computed light spaces for each cascade
            mat4 lightSpace[ShadowMapFrameBuffer::CASCADED_SHADOWMAP_MAX_LAYERS];
            // For cascaded shadow map, the last computed cascade split depth for each cascade
            float splitDepth[ShadowMapFrameBuffer::CASCADED_SHADOWMAP_MAX_LAYERS];
        } frameData[MAX_FRAMES_IN_FLIGHT];

        void update(uint32_t currentFrame) override;

        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

        void loadShaders() override;

        void cleanupImagesResources() override;

    public:
        ShadowMapRenderer(const ShadowMapRenderer &) = delete;

        ShadowMapRenderer &operator=(const ShadowMapRenderer &) = delete;

        ShadowMapRenderer(const ShadowMapRenderer &&) = delete;

        ShadowMapRenderer &&operator=(const ShadowMapRenderer &&) = delete;
    };

} // namespace z0
