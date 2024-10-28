module;
#include "z0/libraries.h"
#include <volk.h>

export module z0:ModelsRenderer;

import :Constants;
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
        virtual void addNode(const shared_ptr<Node> &node, uint32_t currentFrame);

        // Remove a model to the scene and update descriptor set
        virtual void removeNode(const shared_ptr<Node> &node, uint32_t currentFrame);

        // Change the active camera, disable the previous camera
        virtual void activateCamera(const shared_ptr<Camera> &camera, uint32_t currentFrame);

        // Cleanup all Vulkan resources
        void cleanup() override;

        // Get the current scene camera
        [[nodiscard]] inline shared_ptr<Camera> getCamera(const uint32_t currentFrame) const { return frameData[currentFrame].currentCamera; }

    protected:
        struct FrameData {
            // Currently active camera, first camera added to the scene or the last activated
            shared_ptr<Camera> currentCamera{nullptr};
            // All the models of the scene
            list<shared_ptr<MeshInstance>> models{};
            // Data for all the models of the scene, one buffer for all the models
            // https://docs.vulkan.org/samples/latest/samples/performance/descriptor_management/README.html
            unique_ptr<Buffer> modelUniformBuffer;
            // Depth testing multi sampled off-screen buffer
            shared_ptr<DepthFrameBuffer> depthFrameBuffer;
            // Current viewport to reset the viewport size if removed from the scene tree
            shared_ptr<Viewport> currentViewport{nullptr};
        };
        vector<FrameData> frameData;

        ModelsRenderer(Device &device, const string &shaderDirectory, vec3 clearColor);

        // A model is currently been added to the scene, called before updating the descriptor set
        virtual void addingModel(const shared_ptr<MeshInstance>& meshInstance, uint32_t modelIndex, uint32_t currentFrame) {
        }

        // A model had been added to the scene, called after updating the descriptor set
        virtual void addedModel(const shared_ptr<MeshInstance>& meshInstance, uint32_t currentFrame) {
        }

        // A model is currently been removed to the scene, called before updating the descriptor set
        virtual void removingModel(const shared_ptr<MeshInstance>& meshInstance, uint32_t currentFrame) {
        }

        // Set the initial states of the dynamic rendering
        void setInitialState(VkCommandBuffer commandBuffer, uint32_t currentFrame) const;

    public:
        ModelsRenderer(const ModelsRenderer &) = delete;

        ModelsRenderer &operator=(const ModelsRenderer &) = delete;
    };
}
