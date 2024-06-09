#pragma once

namespace z0 {

    class SceneRenderer: public BaseModelsRenderer {
    public:
        SceneRenderer(const Device& device, const string& shaderDirectory);

        shared_ptr<ColorFrameBufferHDR>& getColorAttachement() { return colorFrameBufferHdr; }
        VkImage getImage() const override { return colorFrameBufferHdr->getImage(); }
        VkImageView getImageView() const override { return colorFrameBufferHdr->getImageView(); }

        void cleanup() override;
        void addNode(const shared_ptr<Node>& node) override;
        void removeNode(const shared_ptr<Node>& node) override;

    protected:
        void addingModel(MeshInstance* meshInstance, uint32_t modelIndex) override;
        void addedModel(MeshInstance* meshInstance) override;
        void removingModel(MeshInstance* meshInstance) override;

    private:
        struct GobalUniformBuffer {
            mat4 projection{1.0f};
            mat4 view{1.0f};
            vec4 ambient{ 1.0f, 1.0f, 1.0f, 1.0f }; // RGB + Intensity;
            alignas(16) vec3 cameraPosition;
        };
        struct ModelUniformBuffer {
            mat4 matrix;
        };
        struct MaterialUniformBuffer {
            alignas(4) int transparency{TRANSPARENCY_DISABLED};
            alignas(4) float alphaScissor{0.1};
            alignas(4) int32_t diffuseIndex{-1};
            alignas(4) int32_t specularIndex{-1};
            alignas(4) int32_t normalIndex{-1};
            alignas(16) vec4 albedoColor{0.5,0.5,0.5,1.0};
            alignas(4) float shininess{32.0f};
            alignas(16) float parameters[2];
        };

        // Indices of each models datas in the models uniform buffer
        map<Node::id_t, uint32_t> modelsIndices {};
        // All non-transparents models
        list<MeshInstance*> opaquesModels {};
        // Size of the model uniform buffer
        static constexpr VkDeviceSize modelUniformBufferSize { sizeof(ModelUniformBuffer) };
        // Currently allocated model uniform buffer count
        uint32_t modelUniformBufferCount {0};
        // All the materials of the scene
        list<Material*> materials;
        // Indices of each material in the materials uniform buffer
        map<Resource::id_t, uint32_t> materialsIndices {};
        // Datas for all the materials of the scene, one buffer for all the materials
        vector<unique_ptr<Buffer>> materialsUniformBuffers{MAX_FRAMES_IN_FLIGHT};
        // Size of the above uniform buffers
        static constexpr VkDeviceSize materialUniformBufferSize {  sizeof(MaterialUniformBuffer) };
        // Currently allocated material uniform buffer count
        uint32_t materialUniformBufferCount {0};
        // All material shaders
        map<string, unique_ptr<Shader>> materialShaders;
        // All the images used in the scene
        list<Image*> images;
        // Indices of each images in the descriptor binding
        map<Resource::id_t, int32_t> imagesIndices {};
        // Maximum number of images supported by this renderer
        static constexpr uint32_t MAX_IMAGES = 200;
        // Images infos for descriptor sets, pre-filled with blank images
        array<VkDescriptorImageInfo, MAX_IMAGES> imagesInfo;
        // Default blank image
        unique_ptr<Image> blankImage{nullptr};
        // Default blank image raw datas
        vector<unsigned char> blankImageData;
         // Material for outlining models
        const uint32_t OUTLINE_MATERIAL_INDEX{0};
        shared_ptr<ShaderMaterial> outlineMaterial;
        // For rendering an optional skybox
        unique_ptr<SkyboxRenderer> skyboxRenderer{nullptr};

        // Offscreen frame buffers attachements
        ColorFrameBuffer                colorFrameBufferMultisampled;
        shared_ptr<ColorFrameBufferHDR> colorFrameBufferHdr;
        shared_ptr<DepthFrameBuffer>    resolvedDepthFrameBuffer;

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

    public:
        SceneRenderer(const SceneRenderer&) = delete;
        SceneRenderer &operator=(const SceneRenderer&) = delete;
    };

}