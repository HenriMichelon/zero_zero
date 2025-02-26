/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/vulkan.h"
#include "z0/libraries.h"

export module z0.vulkan.ShadowMapRenderer;

import z0.Constants;
import z0.FrustumCulling;

import z0.nodes.Camera;
import z0.nodes.DirectionalLight;
import z0.nodes.Light;
import z0.nodes.MeshInstance;
import z0.nodes.OmniLight;
import z0.nodes.SpotLight;

import z0.resources.Mesh;

import z0.vulkan.Buffer;
import z0.vulkan.ColorFrameBuffer;
import z0.vulkan.Descriptors;
import z0.vulkan.Device;
import z0.vulkan.Renderer;
import z0.vulkan.Renderpass;
import z0.vulkan.ShadowMapFrameBuffer;

export namespace z0 {

    /*
     * Shadow map renderer, one per light
     */
    class ShadowMapRenderer : public Renderpass, public Renderer {
    public:
        ShadowMapRenderer(Device &device, const shared_ptr<Light>&light);

        void loadScene(const list<shared_ptr<MeshInstance>> &meshes);

        [[nodiscard]] inline auto getLightSpace(const uint32_t index, const uint32_t currentFrame) const {
            return frameData.at(currentFrame).lightSpace[index];
        }

        inline void activateCamera(const shared_ptr<Camera>& camera, const uint32_t currentFrame) {
            frameData.at(currentFrame).currentCamera = camera;
        }

        [[nodiscard]] inline const auto& getLight() const { return light; }

        [[nodiscard]] inline auto isCascaded() const { return light->getLightType() == Light::LIGHT_DIRECTIONAL; }

        [[nodiscard]] inline auto isCubemap() const { return light->getLightType() == Light::LIGHT_OMNI; }

        [[nodiscard]] inline auto getLightPosition() const { return light->getPositionGlobal(); }

        [[nodiscard]] inline auto getCascadesCount(const uint32_t currentFrame) const {
            return frameData.at(currentFrame).cascadesCount;
        }

        [[nodiscard]] inline auto getCascadeSplitDepth(const uint32_t index, const uint32_t currentFrame) const {
            return frameData.at(currentFrame).splitDepth[index];
        }

        [[nodiscard]] inline const auto& getShadowMap(const uint32_t currentFrame) const {
            return frameData.at(currentFrame).shadowMap;
        }

        [[nodiscard]] inline auto getFarPlane() const { return reinterpret_pointer_cast<OmniLight>(light)->getRange(); }

        void cleanup() override;

        ~ShadowMapRenderer() override;

    private:
        struct GlobalBuffer {
            alignas(16) vec3 lightPosition;
            alignas(4) float farPlane;
            mat4 lightSpace[6];
        };

        struct PushConstants {
            mat4 model;
            alignas(4) uint32_t lightSpaceIndex;
            alignas(4) uint32_t transparency;
        };
        static constexpr auto PUSHCONSTANTS_SIZE{sizeof(PushConstants)};
        static constexpr  VkPushConstantRange pushConstantRange {
            .stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS,
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

        struct FrameData {
            // Scene current camera
            shared_ptr<Camera> currentCamera{nullptr};
            // All the models of the scene
            list<shared_ptr<MeshInstance>> models{};
            // The destination frame buffer
            shared_ptr<ShadowMapFrameBuffer> shadowMap;
            // Number of cascades
            uint32_t cascadesCount{1};
            // Last computed light spaces for each cascade / cubemap face
            mat4 lightSpace[6];
            Frustum frustum[6];
            vec3 previousPosition{0.0f};
            mat4 previousProjection{1.0f};
            // For cascaded shadow map, the last computed cascade split depth for each cascade
            float splitDepth[ShadowMapFrameBuffer::CASCADED_SHADOWMAP_MAX_LAYERS];
            // Global UBO in GPU memory
            unique_ptr<Buffer> globalBuffer;
        };
        vector<FrameData> frameData;

        // The light we render the shadow map for
        const shared_ptr<Light> light;

        void update(uint32_t currentFrame) override;

        void drawFrame(uint32_t currentFrame, bool isLast);

        void loadShaders() override;

        void cleanupImagesResources() override;

        void createDescriptorSetLayout() override;

        void createOrUpdateDescriptorSet(bool create) override;

        vector<VkCommandBuffer> getCommandBuffers(uint32_t currentFrame) const override;

    public:
        ShadowMapRenderer(const ShadowMapRenderer &) = delete;

        ShadowMapRenderer &operator=(const ShadowMapRenderer &) = delete;

        ShadowMapRenderer(const ShadowMapRenderer &&) = delete;

        ShadowMapRenderer &&operator=(const ShadowMapRenderer &&) = delete;
    };

} // namespace z0
