module;
#include "z0/libraries.h"
#include <volk.h>

export module Z0:ModelsRenderer;

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
        virtual void addNode(const shared_ptr<Node>& node) {
            if (auto* camera = dynamic_cast<Camera*>(node.get())) {
                if (currentCamera == nullptr) {
                    currentCamera = camera;
                    currentCamera->_setActive(true);
                    //log("Using camera", currentCamera->toString());
                }
            }
            else if (auto* meshInstance = dynamic_cast<MeshInstance*>(node.get())) {
                if (meshInstance->isValid()) {
                    const auto index = models.size();
                    models.push_back(meshInstance);
                    addingModel(meshInstance, index);
                    descriptorSetNeedUpdate = true;
                    createOrUpdateResources();
                    addedModel(meshInstance);
                }
            }
            else if (auto* viewport = dynamic_cast<Viewport*>(node.get())) {
                currentViewport = viewport;
                //log("Using viewport", currentViewport->toString());
            }
        }

        // Remove a model to the scene and update descriptor set
        virtual void removeNode(const shared_ptr<Node>& node) {
            if (auto* camera = dynamic_cast<Camera*>(node.get())) {
                if (camera == currentCamera) {
                    currentCamera->_setActive(false);
                    currentCamera = nullptr;
                }
            }
            else if (auto* meshInstance = dynamic_cast<MeshInstance*>(node.get())) {
                auto it = find(models.begin(), models.end(), meshInstance);
                if (it != models.end()) {
                    models.erase(it);
                    removingModel(meshInstance);
                }
                descriptorSetNeedUpdate = true;
            }
            else if (const auto* viewport = dynamic_cast<Viewport*>(node.get())) {
                if (currentViewport == viewport) {
                    currentViewport = nullptr;
                }
            }
        }

        // Change the active camera, disable the previous camera
        void activateCamera(Camera* camera) {
            if (currentCamera != nullptr) currentCamera->_setActive(false);
            if (camera == nullptr) {
                currentCamera = nullptr;
            }
            else {
                currentCamera = camera;
                currentCamera->_setActive(true);
            }
        }

        void activateCamera(const shared_ptr<Camera>& camera) { activateCamera(camera.get()); }

        // Cleanup all Vulkan resources
        void cleanup() override {
            depthFrameBuffer->cleanupImagesResources();
            cleanupImagesResources();
            modelUniformBuffers.clear();
            Renderpass::cleanup();
        }

        // Get the current scene camera
        [[nodiscard]] Camera* getCamera() const { return currentCamera; }

    protected:
        // Currently active camera, first camera added to the scene or the last activated
        Camera* currentCamera{nullptr};
        // All the models of the scene
        list<MeshInstance*> models{};
        // Datas for all the models of the scene, one buffer for all the models
        // https://docs.vulkan.org/samples/latest/samples/performance/descriptor_management/README.html
        vector<unique_ptr<Buffer>> modelUniformBuffers{MAX_FRAMES_IN_FLIGHT};
        // Depth testing multi sampled off-screen buffer
        shared_ptr<DepthFrameBuffer> depthFrameBuffer;
        // Current viewport to reset the viewport size if removed from the scene tree
        Viewport* currentViewport{nullptr};

        ModelsRenderer(Device& device, const string& shaderDirectory):
            Renderpass(device, shaderDirectory) {
        }

        // A model is currently been added to the scene, called before updating the descriptor set
        virtual void addingModel(MeshInstance* meshInstance, uint32_t modelIndex) {
        };

        // A model had been added to the scene, called after updating the descriptor set
        virtual void addedModel(MeshInstance* meshInstance) {
        };

        // A model is currently been removed to the scene, called before updating the descriptor set
        virtual void removingModel(MeshInstance* meshInstance) {
        };

        // Set the initial states of the dynamic rendering
        void setInitialState(VkCommandBuffer commandBuffer) const {
            bindShaders(commandBuffer);
            if (currentViewport != nullptr) {
                const VkViewport viewport{
                    .x = 0.0f,
                    .y = 0.0f,
                    .width = static_cast<float>(getDevice().getSwapChainExtent().width),
                    .height = static_cast<float>(getDevice().getSwapChainExtent().height),
                    .minDepth = 0.0f,
                    .maxDepth = 1.0f
                };
                vkCmdSetViewportWithCount(commandBuffer, 1, &viewport);
                const VkRect2D scissor{
                    .offset = {
                        static_cast<int32_t>(currentViewport->getViewportPosition().x),
                        static_cast<int32_t>(currentViewport->getViewportPosition().y)
                    },
                    .extent = {
                        static_cast<uint32_t>(currentViewport->getViewportSize().x),
                        static_cast<uint32_t>(currentViewport->getViewportSize().y)
                    },
                };
                vkCmdSetScissorWithCount(commandBuffer, 1, &scissor);
            }
            else {
                setViewport(commandBuffer, getDevice().getSwapChainExtent().width,
                            getDevice().getSwapChainExtent().height);
            }

            vkCmdSetRasterizationSamplesEXT(commandBuffer, device.getSamples());

            constexpr VkBool32 color_blend_enables[] = {VK_FALSE};
            vkCmdSetColorBlendEnableEXT(commandBuffer, 0, 1, color_blend_enables);
            vkCmdSetAlphaToCoverageEnableEXT(commandBuffer, VK_TRUE);

            const auto vertexBinding = Mesh::_getBindingDescription();
            const auto vertexAttribute = Mesh::_getAttributeDescription();
            vkCmdSetVertexInputEXT(commandBuffer,
                                   vertexBinding.size(),
                                   vertexBinding.data(),
                                   vertexAttribute.size(),
                                   vertexAttribute.data());
        }

    public:
        ModelsRenderer(const ModelsRenderer&) = delete;
        ModelsRenderer& operator=(const ModelsRenderer&) = delete;
    };
}
