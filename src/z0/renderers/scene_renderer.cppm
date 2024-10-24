module;
#include <volk.h>
#include "z0/libraries.h"

export module z0:SceneRenderer;

import :Constants;
import :ModelsRenderer;
import :Device;
import :ColorFrameBufferHDR;
import :Descriptors;
import :Node;
import :MeshInstance;
import :ShadowMapRenderer;
import :SkyboxRenderer;
import :Environment;
import :Material;
import :Resource;
import :Buffer;
import :Shader;
import :Image;
import :Cubemap;
import :Light;
import :DirectionalLight;
import :OmniLight;
import :SpotLight;
import :ColorFrameBuffer;
import :DepthFrameBuffer;
import :ShadowMapFrameBuffer;
import :Skybox;
import :SampledFrameBuffer;

namespace z0 {

    /**
     * Main renderer
     */
    export class SceneRenderer : public ModelsRenderer {
    public:
        SceneRenderer(Device &device, const string &shaderDirectory, vec3 clearColor);

        [[nodiscard]] inline const vector<shared_ptr<ColorFrameBufferHDR>> &getColorAttachments() const { return colorFrameBufferHdr; }

        [[nodiscard]] vector<ColorFrameBufferHDR*> getSampledAttachments() const;

        [[nodiscard]] inline VkImage getImage(const uint32_t currentFrame) const override { return colorFrameBufferHdr[currentFrame]->getImage(); }

        [[nodiscard]] inline VkImageView getImageView(const uint32_t currentFrame) const override { return colorFrameBufferHdr[currentFrame]->getImageView(); }

        inline void setShadowCasting(const bool enable) { enableShadowMapRenders = enable; }

        void cleanup() override;

        void addNode(const shared_ptr<Node> &node, uint32_t currentFrame) override;

        void removeNode(const shared_ptr<Node> &node, uint32_t currentFrame) override;

        void preUpdateScene(uint32_t currentFrame);

        void postUpdateScene(uint32_t currentFrame);

        void addingModel(MeshInstance *meshInstance, uint32_t modelIndex, uint32_t currentFrame) override;

        void addedModel(MeshInstance *meshInstance, uint32_t currentFrame) override;

        void removingModel(MeshInstance *meshInstance, uint32_t currentFrame) override;

        void loadShadersMaterials(const ShaderMaterial *material, uint32_t currentFrame);

        void activateCamera(Camera *camera, uint32_t currentFrame) override;

    private:
        struct GobalBuffer {
            mat4 projection{1.0f};
            mat4 view{1.0f};
            vec4 ambient{1.0f, 1.0f, 1.0f, 1.0f}; // RGB + Intensity;
            alignas(16) vec3 cameraPosition;
            alignas(4) uint32_t lightsCount{0};
        };

        struct ModelBuffer {
            mat4 matrix{};
        };

        struct MaterialBuffer {
            alignas(4) int transparency{TRANSPARENCY_DISABLED};
            alignas(4) float alphaScissor{0.1f};
            alignas(4) int32_t diffuseIndex{-1};
            alignas(4) int32_t specularIndex{-1};
            alignas(4) int32_t normalIndex{-1};
            alignas(16) vec4 albedoColor{0.5f, 0.5f, 0.5f, 1.0f};
            alignas(4) float shininess{32.0f};
            alignas(4) bool hasTransform{false};
            alignas(8) vec2 textureOffset{0.0f, 0.0f};
            alignas(8) vec2 textureScale{1.0f, 1.0f};
            alignas(16) vec4 parameters[ShaderMaterial::MAX_PARAMETERS];
        };

        struct LightBuffer {
            // light params
            alignas(4) int32_t type{Light::LIGHT_UNKNOWN}; // Light::LightType
            alignas(16) vec3 position{0.0f, 0.0f, 0.0f};
            alignas(16) vec3 direction{0.0f};
            alignas(16) vec4 color{1.0f, 1.0f, 1.0f, 1.0f}; // RGB + Intensity;
            alignas(4) float specular{1.0f};
            alignas(4) float range{0.0f};
            alignas(4) float cutOff{0.0f};
            alignas(4) float outerCutOff{0.0f};
            // shadow map params
            alignas(4) int32_t mapIndex{-1};
            alignas(4) float farPlane{0.0};
            alignas(4) uint32_t cascadesCount{0};
            alignas(16) vec4 cascadeSplitDepth;
            mat4 lightSpace[6];
        };

        struct PushConstants {
            alignas(4) int modelIndex;
            alignas(4) int materialIndex;
        };
        static constexpr uint32_t PUSHCONSTANTS_SIZE{sizeof(PushConstants)};
        static constexpr VkPushConstantRange pushConstantRange {
            .stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS,
            .offset = 0,
            .size = PUSHCONSTANTS_SIZE
        };

        // Maximum number of materials supported by this renderer
        static constexpr uint32_t MAX_MATERIALS{200};
        // Maximum number of shadow maps supported by this renderer
        static constexpr uint32_t MAX_SHADOW_MAPS{10};
        // Maximum number of images supported by this renderer
        static constexpr uint32_t MAX_IMAGES{200};
        // Constant depth bias factor (always applied)
        static constexpr float depthBiasConstant = 0.0f;
        // Slope depth bias factor, applied depending on polygon's slope
        static constexpr float depthBiasSlope = 1.0f;

        // Size of each model matrix buffer
        static constexpr VkDeviceSize MODEL_BUFFER_SIZE{sizeof(ModelBuffer)};
        // Size of each material buffer
        static constexpr VkDeviceSize MATERIAL_BUFFER_SIZE{sizeof(MaterialBuffer)};
        // Size of the point light uniform buffers
        static constexpr VkDeviceSize LIGHT_BUFFER_SIZE{sizeof(LightBuffer)};

        struct FrameData {
            // Indices of each model data in the models uniform buffer
            map<Node::id_t, uint32_t> modelsIndices{};
            // All non-transparent models
            list<MeshInstance *> opaquesModels{};
            // Currently allocated model uniform buffer count
            uint32_t modelBufferCount{0};

            // All materials used in the scene, used to update the buffer in GPU memory
            list<shared_ptr<Material>> materials;
            // Vector to track free indices
            vector<Resource::id_t> materialsIndicesAllocation;
            // Indices of each material in the buffer
            map<Resource::id_t, int32_t> materialsIndices{};
            // Data for all the materials of the scene, one buffer for all the materials
            unique_ptr<Buffer> materialsBuffer;

            // Images infos for descriptor sets, pre-filled with blank images
            array<VkDescriptorImageInfo, MAX_SHADOW_MAPS> shadowMapsInfo;
            // Images infos for descriptor sets, pre-filled with blank cubemaps
            array<VkDescriptorImageInfo, MAX_SHADOW_MAPS> shadowMapsCubemapInfo;

            // All material shaders
            map<string, unique_ptr<Shader>> materialShaders;
            // All the images used in the scene
            list<Image *> images;
            // Indices of each images in the descriptor binding
            map<Resource::id_t, int32_t> imagesIndices{};
            // Images infos for descriptor sets, pre-filled with blank images
            array<VkDescriptorImageInfo, MAX_IMAGES> imagesInfo;
            // For rendering an optional skybox
            unique_ptr<SkyboxRenderer> skyboxRenderer{nullptr};
            // Environment parameters for the current scene
            Environment *currentEnvironment{nullptr};

            // All lights
            vector<const Light *> lights;
            // Lights & shadow maps UBO
            unique_ptr<Buffer> lightBuffer;
            // Currently allocated point light uniform buffer count
            uint32_t lightBufferCount{0};

            // Offscreen frame buffers attachments
            unique_ptr<ColorFrameBuffer>    colorFrameBufferMultisampled;
            shared_ptr<DepthFrameBuffer>    resolvedDepthFrameBuffer;
        };
        vector<FrameData> frameData;

        // Enable or disable shadow casting (for the editor)
        bool enableShadowMapRenders{true};
        // One renderer per shadow map
        map<const Light*, shared_ptr<ShadowMapRenderer>> shadowMapRenderers;
        // Default blank image (for textures and shadow mapping)
        unique_ptr<Image> blankImage{nullptr};
        // Default blank cubemap (for omni shadow mapping)
        unique_ptr<Cubemap> blankCubemap{nullptr};

        vector<shared_ptr<ColorFrameBufferHDR>> colorFrameBufferHdr;

        void update(uint32_t currentFrame) override;

        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

        void createDescriptorSetLayout() override;

        void createOrUpdateDescriptorSet(bool create) override;

        void loadShaders() override;

        void createImagesResources() override;

        void cleanupImagesResources() override;

        void recreateImagesResources() override;

        void beginRendering(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

        void endRendering(VkCommandBuffer commandBuffer, uint32_t currentFrame, bool isLast) override;

        void addMaterial(const shared_ptr<Material> &material, uint32_t currentFrame);

        void removeMaterial(const shared_ptr<Material> &material, uint32_t currentFrame);

        void addImage(const shared_ptr<Image> &image, uint32_t currentFrame);

        void removeImage(const shared_ptr<Image> &image, uint32_t currentFrame);

        void drawModels(VkCommandBuffer commandBuffer, uint32_t currentFrame, const list<MeshInstance *> &modelsToDraw);

        [[nodiscard]] shared_ptr<ShadowMapRenderer> findShadowMapRenderer(const Light *light) const {
            return shadowMapRenderers.at(light);
        }

        void enableLightShadowCasting(const Light *light);

        void disableLightShadowCasting(const Light *light);

    public:
        SceneRenderer(const SceneRenderer &) = delete;

        SceneRenderer &operator=(const SceneRenderer &) = delete;
    };
} // namespace z0
