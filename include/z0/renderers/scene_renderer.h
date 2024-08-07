#pragma once

namespace z0 {

    /**
     * Main renderer
     */
    class SceneRenderer: public ModelsRenderer {
    public:
        SceneRenderer(Device& device, const string& shaderDirectory);

        [[nodiscard]] shared_ptr<ColorFrameBufferHDR>& getColorAttachment() { return colorFrameBufferHdr; }
        [[nodiscard]] VkImage getImage() const override { return colorFrameBufferHdr->getImage(); }
        [[nodiscard]] VkImageView getImageView() const override { return colorFrameBufferHdr->getImageView(); }
        void setShadowCasting(bool);

        void cleanup() override;
        void addNode(const shared_ptr<Node>& node) override;
        void removeNode(const shared_ptr<Node>& node) override;
        void preUpdateScene();
        void postUpdateScene();

    protected:
        void addingModel(MeshInstance* meshInstance, uint32_t modelIndex) override;
        void addedModel(MeshInstance* meshInstance) override;
        void removingModel(MeshInstance* meshInstance) override;
        void loadShadersMaterials(ShaderMaterial* material);

    private:
        struct DirectionalLightUniform {
            alignas(16) vec3 direction  { 0.0f, 0.0f, 0.0f };
            alignas(16) vec4 color      { 0.0f, 0.0f, 0.0f, 0.0f }; // RGB + Intensity;
            alignas(4)  float specular  { 1.0f };
        };
        struct GobalUniformBuffer {
            mat4 projection { 1.0f };
            mat4 view       { 1.0f} ;
            vec4 ambient    { 1.0f, 1.0f, 1.0f, 1.0f }; // RGB + Intensity;
            alignas(16) vec3                    cameraPosition;
            alignas(16) DirectionalLightUniform directionalLight;
            alignas(4)  bool                    haveDirectionalLight{ false };
            alignas(4)  uint32_t                pointLightsCount    { 0 };
            alignas(4)  uint32_t                shadowMapsCount     { 0 };
        };
        struct ShadowMapUniformBuffer {
            mat4    lightSpace;
            alignas(16) vec3 lightPos;
        };
        struct ModelUniformBuffer {
            mat4    matrix;
        };
        struct MaterialUniformBuffer {
            alignas(4)  int     transparency    { TRANSPARENCY_DISABLED };
            alignas(4)  float   alphaScissor    { 0.1f };
            alignas(4)  int32_t diffuseIndex    { -1 };
            alignas(4)  int32_t specularIndex   { -1 };
            alignas(4)  int32_t normalIndex     { -1 };
            alignas(16) vec4    albedoColor     { 0.5f, 0.5f, 0.5f, 1.0f };
            alignas(4)  float   shininess       { 32.0f };
            alignas(4)  bool    hasTransform    { false };
            alignas(8)  vec2    textureOffset   { 0.0f, 0.0f };
            alignas(8)  vec2    textureScale    { 1.0f, 1.0f };
            alignas(16) vec4    parameters[ShaderMaterial::MAX_PARAMETERS];
        };
         struct PointLightUniformBuffer {
            alignas(16) vec3    position    { 0.0f, 0.0f, 0.0f };
            alignas(16) vec4    color       { 1.0f, 1.0f, 1.0f, 1.0f }; // RGB + Intensity;
            alignas(4)  float   specular    { 1.0f };
            alignas(4)  float   constant    { 1.0f };
            alignas(4)  float   linear      { 0.0f };
            alignas(4)  float   quadratic   { 0.0f };
            alignas(4)  bool    isSpot      { false };
            alignas(16) vec3    direction   { vec3{0.f, .0f, .0f} };
            alignas(4)  float   cutOff      { cos(radians(10.0f)) };
            alignas(4)  float   outerCutOff { cos(radians(15.0f)) };
        };

        // Indices of each models datas in the models uniform buffer
        map<Node::id_t, uint32_t>       modelsIndices {};
        // All non-transparents models
        list<MeshInstance*>             opaquesModels {};
        // Size of the model uniform buffer
        static constexpr VkDeviceSize   modelUniformBufferSize { sizeof(ModelUniformBuffer) };
        // Currently allocated model uniform buffer count
        uint32_t                        modelUniformBufferCount {0};
        // All the materials of the scene
        list<Material*>             materials;
        // Indices of each material in the materials uniform buffer
        map<Resource::id_t, uint32_t>   materialsIndices {};
        // Datas for all the materials of the scene, one buffer for all the materials
        vector<unique_ptr<Buffer>>      materialsUniformBuffers{MAX_FRAMES_IN_FLIGHT};
        // Size of the above uniform buffers
        static constexpr VkDeviceSize   materialUniformBufferSize {  sizeof(MaterialUniformBuffer) };
        // Currently allocated material uniform buffer count
        uint32_t                        materialUniformBufferCount {0};

        // Enable or disable shadow casting (for the editor)
        bool                                    enableShadowMapRenders{true};
        // One renderer per shadow map
        vector<shared_ptr<ShadowMapRenderer>>   shadowMapRenderers;
        // One buffer per shadow map with light information
        vector<unique_ptr<Buffer>>              shadowMapsUniformBuffers{MAX_FRAMES_IN_FLIGHT};
        // Size of the above uniform buffers
        static constexpr VkDeviceSize           shadowMapUniformBufferSize {  sizeof(ShadowMapUniformBuffer) };
        // Currently allocated material uniform buffer count
        uint32_t                                shadowMapUniformBufferCount {0};
        // Maximum number of shadow maps supported by this renderer
        static constexpr uint32_t               MAX_SHADOW_MAPS = 100;
        // Images infos for descriptor sets, pre-filled with blank images
        array<VkDescriptorImageInfo, MAX_SHADOW_MAPS> shadowMapsInfo;

        // All material shaders
        map<string, unique_ptr<Shader>>     materialShaders;
        // All the images used in the scene
        list<Image*>                        images;
        // Indices of each images in the descriptor binding
        map<Resource::id_t, int32_t>        imagesIndices {};
        // Maximum number of images supported by this renderer
        static constexpr uint32_t           MAX_IMAGES = 200;
        // Images infos for descriptor sets, pre-filled with blank images
        array<VkDescriptorImageInfo, MAX_IMAGES> imagesInfo;
        // Default blank image
        unique_ptr<Image>                   blankImage{nullptr};
        // Default blank image raw datas
        vector<unsigned char>               blankImageData;
        // For rendering an optional skybox
        unique_ptr<SkyboxRenderer>          skyboxRenderer{nullptr};
        // Evironement parameters for the current scene
        Environment*                        currentEnvironment{nullptr};
        
        // One and only one directional light per scene
        DirectionalLight*                   directionalLight{nullptr};
        // Omni and Spot lights
        vector<OmniLight*>                  omniLights;
        // Omni and Spot lights UBOs
        vector<unique_ptr<Buffer>>          pointLightUniformBuffers{MAX_FRAMES_IN_FLIGHT};
        // Size of the above uniform buffers
        static constexpr VkDeviceSize       pointLightUniformBufferSize {  sizeof(PointLightUniformBuffer) };
        // Currently allocated point light uniform buffer count
        uint32_t                            pointLightUniformBufferCount{0};

        // Offscreen frame buffers attachements
        ColorFrameBuffer                    colorFrameBufferMultisampled;
        shared_ptr<ColorFrameBufferHDR>     colorFrameBufferHdr;
        shared_ptr<DepthFrameBuffer>        resolvedDepthFrameBuffer;

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

        void addImage(const shared_ptr<Image>& image);
        void removeImage(const shared_ptr<Image>& image);
        void drawModels(VkCommandBuffer commandBuffer, uint32_t currentFrame, const list<MeshInstance*>& modelsToDraw);
        [[nodiscard]] shared_ptr<ShadowMapRenderer> findShadowMapRenderer(const Light* light) const;

    public:
        SceneRenderer(const SceneRenderer&) = delete;
        SceneRenderer &operator=(const SceneRenderer&) = delete;
    };

}