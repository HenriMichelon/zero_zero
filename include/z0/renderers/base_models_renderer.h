#pragma once

namespace z0 {

    class BaseModelsRenderer: public BaseRenderpass, public BaseRenderer {
    public:
        // Add a model to the scene and update descriptor set
        virtual void addNode(const shared_ptr<Node>& node);
        // Remove a model to the scene and update descriptor set
        virtual void removeNode(const shared_ptr<Node>& node);
        // Change the active camera, disable the previous camera
        void activateCamera(Camera* camera);
        void activateCamera(const shared_ptr<Camera>& camera) { activateCamera(camera.get()); }
        // Cleanup all Vulkan ressources
        void cleanup() override;
        // Get the current scene camera
        Camera* getCamera() const { return currentCamera; }

    protected:
        // Currently active camera, first camera added to the scene or the last activated
        Camera* currentCamera {nullptr};
        // All the models of the scene
        list<MeshInstance*> models {};
        // Datas for all the models of the scene, one buffer for all the models
        // https://docs.vulkan.org/samples/latest/samples/performance/descriptor_management/README.html
        vector<unique_ptr<Buffer>> modelUniformBuffers{MAX_FRAMES_IN_FLIGHT};
        // Depth testing multisampled off screen buffer
        shared_ptr<DepthFrameBuffer> depthFrameBuffer;

        BaseModelsRenderer(Device& device, const string& shaderDirectory);

        // A model is currently been added to the scene, called before updating the descriptor set
        virtual void addingModel(MeshInstance* meshInstance, uint32_t modelIndex) {};
        // A model is have been added to the scene, called after updating the descriptor set
        virtual void addedModel(MeshInstance* meshInstance) {};
        // A model is currently been removed to the scene, called before updating the descriptor set
        virtual void removingModel(MeshInstance* meshInstance) {};
        // Set the initial states of the dynamic rendering
        void setInitialState(VkCommandBuffer commandBuffer);

    public:
        BaseModelsRenderer(const BaseModelsRenderer&) = delete;
        BaseModelsRenderer &operator=(const BaseModelsRenderer&) = delete;
    };

}