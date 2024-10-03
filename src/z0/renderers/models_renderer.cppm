module;
#include "z0/libraries.h"
#include <volk.h>

export module z0:ModelsRenderer;

import :Renderer;
import :Renderpass;
import :Node;
import :Camera;
import :Buffer;
import :MeshInstance;
import :Device;
import :DepthFrameBuffer;
import :Viewport;
import :Mesh;

export namespace z0 {
    /**
     * Base class for renderers drawing a collection of Mesh
     */
    class ModelsRenderer : public Renderpass, public Renderer {
    public:
        // Add a model to the scene and update descriptor set
        virtual void addNode(const shared_ptr<Node> &node);

        // Remove a model to the scene and update descriptor set
        virtual void removeNode(const shared_ptr<Node> &node);

        // Change the active camera, disable the previous camera
        void activateCamera(Camera *camera);

        inline void activateCamera(const shared_ptr<Camera> &camera) { activateCamera(camera.get()); }

        // Cleanup all Vulkan resources
        void cleanup() override;

        // Get the current scene camera
        [[nodiscard]] inline Camera *getCamera() const { return currentCamera; }

    protected:
        // Currently active camera, first camera added to the scene or the last activated
        Camera *currentCamera{nullptr};
        // All the models of the scene
        list<MeshInstance *> models{};
        // Datas for all the models of the scene, one buffer for all the models
        // https://docs.vulkan.org/samples/latest/samples/performance/descriptor_management/README.html
        vector<unique_ptr<Buffer>> modelUniformBuffers{MAX_FRAMES_IN_FLIGHT};
        // Depth testing multi sampled off-screen buffer
        shared_ptr<DepthFrameBuffer> depthFrameBuffer;
        // Current viewport to reset the viewport size if removed from the scene tree
        Viewport *currentViewport{nullptr};

        ModelsRenderer(Device &device, const string &shaderDirectory, vec3 clearColor);

        // A model is currently been added to the scene, called before updating the descriptor set
        virtual void addingModel(MeshInstance *meshInstance, uint32_t modelIndex) {
        }

        // A model had been added to the scene, called after updating the descriptor set
        virtual void addedModel(MeshInstance *meshInstance) {
        }

        // A model is currently been removed to the scene, called before updating the descriptor set
        virtual void removingModel(MeshInstance *meshInstance) {
        }

        // Set the initial states of the dynamic rendering
        void setInitialState(VkCommandBuffer commandBuffer) const;

    public:
        ModelsRenderer(const ModelsRenderer &) = delete;

        ModelsRenderer &operator=(const ModelsRenderer &) = delete;
    };
}
