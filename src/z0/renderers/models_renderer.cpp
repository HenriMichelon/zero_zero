module;
#include "z0/libraries.h"
#include <volk.h>

 module z0;

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
import :ModelsRenderer;

 namespace z0 {

    void ModelsRenderer::addNode(const shared_ptr<Node> &node) {
        if (auto *camera = dynamic_cast<Camera *>(node.get())) {
            if (currentCamera == nullptr) {
                activateCamera(camera);
                //log("Using camera", currentCamera->toString());
            }
        } else if (auto *meshInstance = dynamic_cast<MeshInstance *>(node.get())) {
            if (meshInstance->isValid()) {
                const auto index = models.size();
                models.push_back(meshInstance);
                addingModel(meshInstance, index);
                descriptorSetNeedUpdate = true;
                createOrUpdateResources();
                addedModel(meshInstance);
            }
        } else if (auto *viewport = dynamic_cast<Viewport *>(node.get())) {
            currentViewport = viewport;
            //log("Using viewport", currentViewport->toString());
        }
    }

    void ModelsRenderer::removeNode(const shared_ptr<Node> &node) {
        if (auto *camera = dynamic_cast<Camera *>(node.get())) {
            if (camera == currentCamera) {
                currentCamera->_setActive(false);
                currentCamera = nullptr;
            }
        } else if (auto *meshInstance = dynamic_cast<MeshInstance *>(node.get())) {
            auto it = find(models.begin(), models.end(), meshInstance);
            if (it != models.end()) {
                models.erase(it);
                removingModel(meshInstance);
            }
            descriptorSetNeedUpdate = true;
        } else if (const auto *viewport = dynamic_cast<Viewport *>(node.get())) {
            if (currentViewport == viewport) {
                currentViewport = nullptr;
            }
        }
    }

    void ModelsRenderer::activateCamera(Camera *camera) {
        if (currentCamera != nullptr)
            currentCamera->_setActive(false);
        if (camera == nullptr) {
            currentCamera = nullptr;
        } else {
            currentCamera = camera;
            currentCamera->_setActive(true);
        }
    }

    void ModelsRenderer::cleanup() {
        depthFrameBuffer->cleanupImagesResources();
        cleanupImagesResources();
        modelUniformBuffers.clear();
        Renderpass::cleanup();
    }

    ModelsRenderer::ModelsRenderer(Device &device, const string &shaderDirectory, vec3 clearColor):
        Renderpass(device, shaderDirectory, clearColor) {
    }

    // Set the initial states of the dynamic rendering
    void ModelsRenderer::setInitialState(const VkCommandBuffer commandBuffer) const {
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
        } else {
            setViewport(commandBuffer,
                        getDevice().getSwapChainExtent().width,
                        getDevice().getSwapChainExtent().height);
        }

        vkCmdSetRasterizationSamplesEXT(commandBuffer, device.getSamples());
        constexpr VkBool32 color_blend_enables[] = {VK_FALSE};
        vkCmdSetColorBlendEnableEXT(commandBuffer, 0, 1, color_blend_enables);
        vkCmdSetAlphaToCoverageEnableEXT(commandBuffer, VK_TRUE);

        const auto vertexBinding   = Mesh::_getBindingDescription();
        const auto vertexAttribute = Mesh::_getAttributeDescription();
        vkCmdSetVertexInputEXT(commandBuffer,
                               vertexBinding.size(),
                               vertexBinding.data(),
                               vertexAttribute.size(),
                               vertexAttribute.data());
    }

}
