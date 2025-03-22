/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/vulkan.h"
#include "z0/libraries.h"

export module z0.vulkan.SceneRenderer;

import z0.Constants;
import z0.FrustumCulling;

import z0.nodes.Camera;
import z0.nodes.Node;
import z0.nodes.MeshInstance;
import z0.nodes.Environment;
import z0.nodes.Light;
import z0.nodes.DirectionalLight;
import z0.nodes.OmniLight;
import z0.nodes.SpotLight;
import z0.nodes.Skybox;

import z0.resources.Material;
import z0.resources.Resource;
import z0.resources.Image;
import z0.resources.Cubemap;

import z0.vulkan.Buffer;
import z0.vulkan.ColorFrameBuffer;
import z0.vulkan.ColorFrameBufferHDR;
import z0.vulkan.DebugRenderer;
import z0.vulkan.DepthFrameBuffer;
import z0.vulkan.Device;
import z0.vulkan.Descriptors;
import z0.vulkan.ModelsRenderer;
import z0.vulkan.NormalFrameBuffer;
import z0.vulkan.Shader;
import z0.vulkan.ShadowMapFrameBuffer;
import z0.vulkan.ShadowMapRenderer;
import z0.vulkan.SampledFrameBuffer;
import z0.vulkan.SkyboxRenderer;
import z0.vulkan.Cubemap;
import z0.vulkan.Image;


namespace z0 {

    /*
     * Main renderer
     */
    export class SceneRenderer : public ModelsRenderer {
    public:
        SceneRenderer(Device &device, vec3 clearColor, bool enableDepthPrepass, bool enableNormalPrepass);

        [[nodiscard]] inline const auto& getColorAttachments() const { return colorFrameBufferHdr; }

        [[nodiscard]] inline const auto& getDepthAttachments() const { return resolvedDepthFrameBuffer; }

        [[nodiscard]] inline const auto& getNormalAttachments() const { return resolvedNormalFrameBuffer; }

        [[nodiscard]] inline VkImage getImage(const uint32_t currentFrame) const override { return colorFrameBufferHdr[currentFrame]->getImage(); }

        [[nodiscard]] inline VkImageView getImageView(const uint32_t currentFrame) const override { return colorFrameBufferHdr[currentFrame]->getImageView(); }

        inline auto setShadowCasting(const bool enable) { enableShadowMapRenders = enable; }

        void cleanup() override;

        void addNode(const shared_ptr<Node> &node, uint32_t currentFrame) override;

        void removeNode(const shared_ptr<Node> &node, uint32_t currentFrame) override;

        void preUpdateScene(uint32_t currentFrame);

        void postUpdateScene(uint32_t currentFrame);

        void addingModel(const shared_ptr<MeshInstance>& meshInstance, uint32_t currentFrame) override;

        void removingModel(const shared_ptr<MeshInstance>& meshInstance, uint32_t currentFrame) override;

        void loadShadersMaterials(const shared_ptr<ShaderMaterial>& material, uint32_t currentFrame);

        void activateCamera(const shared_ptr<Camera>& camera, uint32_t currentFrame) override;

    private:
        struct GlobalBuffer {
            mat4                projection{1.0f};
            mat4                view{1.0f};
            alignas(16) vec3    cameraPosition;
            alignas(4) uint32_t lightsCount;
            alignas(16) vec4    ambient; // RGB + Intensity;
            alignas(4) uint32_t ambientIBL; // Only if HDRi skybox
            alignas(8) ivec2   screenSize{0};
        };
        static constexpr auto GLOBAL_BUFFER_SIZE = sizeof(GlobalBuffer);

        struct ModelBuffer {
            mat4 matrix{};
        };

        enum Bindings : uint32_t {
            BINDING_GLOBAL_BUFFER      = 0,
            BINDING_MODELS_BUFFER      = 1,
            BINDING_MATERIALS_BUFFER   = 2,
            BINDING_TEXTURES_BUFFER    = 3,
            BINDING_LIGHTS_BUFFER      = 4,
            BINDING_TEXTURES           = 5,
            BINDING_SHADOW_MAPS        = 6,
            BINDING_SHADOW_CUBEMAPS    = 7,
            BINDING_PBR_ENV_MAP        = 8,
            BINDING_PBR_IRRADIANCE_MAP = 9,
            BINDING_PBR_BRDF_LUT       = 10,
            BINDING_DEPTH_BUFFER       = 11,
            // BINDING_NORMAL_BUFFER      = 12,
        };

        struct MaterialBuffer {
            alignas(16) vec4 albedoColor{0.5f, 0.5f, 0.5f, 1.0f};
            alignas(4) int   transparency{0};
            alignas(4) float alphaScissor{0.1f};
            alignas(4) float metallicFactor{-1.0f}; // -1.0f -> non PBR material
            alignas(4) float roughnessFactor{1.0f};
            alignas(16) vec3 emissiveFactor{0.0f};
            alignas(4) float emissiveStrength{1.0f};
            alignas(4) float normalScale{1.0f};
            alignas(16) vec4 parameters[ShaderMaterial::MAX_PARAMETERS];
        };

        // Material & Texture infos are split in two buffers because of the 64kb buffer limit on some GPU/drivers
        struct TextureInfo {
            alignas(4) int32_t  index{-1};
            alignas(16) mat3x4  transform{1.0f};
        };
        struct TextureBuffer {
            alignas(16) TextureInfo  albedoTexture{};
            alignas(16) TextureInfo  normalTexture{};
            alignas(16) TextureInfo  metallicTexture{};
            alignas(16) TextureInfo  roughnessTexture{};
            alignas(16) TextureInfo  emissiveTexture{};
        };

        struct LightBuffer {
            // light params
            alignas(4) int32_t type{Light::LIGHT_UNKNOWN}; // Light::LightType
            alignas(16) vec3 position{0.0f, 0.0f, 0.0f};
            alignas(16) vec3 direction{0.0f};
            alignas(16) vec4 color{1.0f, 1.0f, 1.0f, 1.0f}; // RGB + Intensity;
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
        static constexpr vector<Resource::id_t>::size_type MAX_MATERIALS{200};
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
        // Size of each material textures buffer
        static constexpr VkDeviceSize TEXTURE_BUFFER_SIZE{sizeof(TextureBuffer)};
        // Size of the point light uniform buffers
        static constexpr VkDeviceSize LIGHT_BUFFER_SIZE{sizeof(LightBuffer)};

        struct FrameData {
            // Indices of each model data in the models uniform buffer
            map<Resource::id_t, uint32_t> meshesIndices{};
            // All non-transparent models
            map<Resource::id_t, list<shared_ptr<MeshInstance>>> opaquesModels{};
            // All transparent models
            map<Resource::id_t, list<shared_ptr<MeshInstance>>> transparentModels{};
            // Currently allocated model uniform buffer count
            uint32_t modelBufferCount{0};
            bool drawOutlines{false};
            unique_ptr<ModelBuffer[]> modelUBOArray;

            // All materials used in the scene, used to update the buffer in GPU memory
            list<shared_ptr<Material>> materials;
            // Rebuild the entier material buffer ?
            bool materialsDirty{false};
            // Material reference counter
            map<Resource::id_t, uint32_t> materialsRefCounter;
            // Indices of each material & texture in the buffers
            map<Resource::id_t, int32_t> materialsIndices{};
            // Data for all the materials of the scene, one buffer for all the materials
            unique_ptr<Buffer> materialsBuffer;
            // Data for all the materials textures of the scene, one buffer for all the textures
            unique_ptr<Buffer> texturesBuffer;

            // Images infos for descriptor sets, pre-filled with blank images
            array<VkDescriptorImageInfo, MAX_SHADOW_MAPS> shadowMapsInfo;
            // Images infos for descriptor sets, pre-filled with blank cubemaps
            array<VkDescriptorImageInfo, MAX_SHADOW_MAPS> shadowMapsCubemapInfo;
            // Image info for depth buffer
            VkDescriptorImageInfo depthBufferInfo;
            // Image info for normal buffer
            // VkDescriptorImageInfo normalBufferInfo;

            // All material shaders
            map<string, unique_ptr<Shader>> materialShaders;
            // All the images used in the scene
            list<shared_ptr<VulkanImage>> images;
            // Indices of each images in the descriptor binding
            map<Resource::id_t, int32_t> imagesIndices{};
            // Images reference counter
            map<Resource::id_t, uint32_t> imagesRefCounter;
            // Images infos for descriptor sets, pre-filled with blank images
            array<VkDescriptorImageInfo, MAX_IMAGES> imagesInfo;
            // For rendering an optional skybox
            unique_ptr<SkyboxRenderer> skyboxRenderer{nullptr};
            // Environment parameters for the current scene
            shared_ptr<Environment> currentEnvironment{nullptr};

            // All lights
            vector<shared_ptr<Light>> lights;
            // Lights & shadow maps UBO
            unique_ptr<Buffer> lightBuffer;
            // Currently allocated point light uniform buffer count
            uint32_t lightBufferCount{0};

            // Offscreen frame buffers attachments
            unique_ptr<ColorFrameBuffer> colorFrameBufferMultisampled;

            // Global UBO in GPU memory
            unique_ptr<Buffer> globalBuffer;

            // Current camera frustum
            Frustum cameraFrustum;
        };
        vector<FrameData> frameData;

        // Enable or disable shadow casting (for the editor)
        bool enableShadowMapRenders{true};
        // Enable the depth pre-pass
        bool enableDepthPrepass;
        // Enable the normal pre-pass
        bool enableNormalPrepass;
        // One renderer per shadow map
        map<shared_ptr<Light>, shared_ptr<ShadowMapRenderer>> shadowMapRenderers;
        // Default blank image (for textures & optional frame buffers)
        shared_ptr<VulkanImage> blankImage{nullptr};
        // Default blank image (for shadow mapping)
        shared_ptr<VulkanImage> blankImageArray{nullptr};
        // Default blank cubemap (for omni shadow mapping)
        shared_ptr<VulkanCubemap> blankCubemap{nullptr};
        // Vertex shader for the depth pre-pass
        unique_ptr<Shader> depthPrepassVertShader;
        // Vertex shader for the normal pre-pass
        unique_ptr<Shader> normalPrepassVertShader;
        // Fragment shader for the normal pre-pass
        unique_ptr<Shader> normalPrepassFragShader;
        // destination frame buffer
        vector<shared_ptr<ColorFrameBufferHDR>> colorFrameBufferHdr;
        // resolved depth buffer destination frame buffer
        vector<shared_ptr<DepthFrameBuffer>> resolvedDepthFrameBuffer;
        // Normal multi sampled off-screen buffer
        vector<shared_ptr<NormalFrameBuffer>> normalFrameBuffer;
        // resolved normal buffer destination frame buffer
        vector<shared_ptr<NormalFrameBuffer>> resolvedNormalFrameBuffer;

        void update(uint32_t currentFrame) override;

        void drawFrame(uint32_t currentFrame, bool isLast) override;

        void createDescriptorSetLayout() override;

        void createOrUpdateDescriptorSet(bool create) override;

        void loadShaders() override;

        void createImagesResources() override;

        void cleanupImagesResources() override;

        void recreateImagesResources() override;

        void beginRendering( uint32_t currentFrame);

        void endRendering(uint32_t currentFrame, bool isLast);

        void addMaterial(const shared_ptr<Material> &material, uint32_t currentFrame);

        void removeMaterial(const shared_ptr<Material> &material, uint32_t currentFrame);

        void addImage(const shared_ptr<Image> &image, uint32_t currentFrame);

        void removeImage(const shared_ptr<Image> &image, uint32_t currentFrame);

        void drawModels(uint32_t currentFrame, const map<Resource::id_t, list<shared_ptr<MeshInstance>>> &modelsToDraw);

        void drawOutlines(uint32_t currentFrame, const map<Resource::id_t, list<shared_ptr<MeshInstance>>> &modelsToDraw);

        void depthPrepass(uint32_t currentFrame, const map<Resource::id_t, list<shared_ptr<MeshInstance>>> &modelsToDraw);

        void normalPrepass(uint32_t currentFrame, const map<Resource::id_t, list<shared_ptr<MeshInstance>>> &modelsToDraw);

        void drawModelsWithoutMaterial(uint32_t currentFrame, const map<Resource::id_t, list<shared_ptr<MeshInstance>>> &modelsToDraw);

        [[nodiscard]] auto findShadowMapRenderer(const shared_ptr<Light>& light) const {
            return shadowMapRenderers.at(light);
        }

        void enableLightShadowCasting(const shared_ptr<Light>&light);

        void disableLightShadowCasting(const shared_ptr<Light>&light);

    public:
        SceneRenderer(const SceneRenderer &) = delete;

        SceneRenderer &operator=(const SceneRenderer &) = delete;
    };
} // namespace z0
