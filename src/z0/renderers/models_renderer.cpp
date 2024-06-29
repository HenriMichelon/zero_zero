#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/resources/image.h"
#include "z0/resources/texture.h"
#include "z0/resources/material.h"
#include "z0/resources/mesh.h"
#include "z0/nodes/node.h"
#include "z0/nodes/camera.h"
#include "z0/nodes/mesh_instance.h"
#include "z0/nodes/viewport.h"
#include "z0/renderers/renderpass.h"
#include "z0/renderers/models_renderer.h"
#include "z0/resources/mesh.h"
#include "z0/descriptors.h"
#endif

namespace z0 {

    ModelsRenderer::ModelsRenderer(Device &dev, const string& sDir):
        Renderpass(dev, sDir) {
    }

    void ModelsRenderer::addNode(const shared_ptr<Node>& node) {
        if (auto* camera = dynamic_cast<Camera*>(node.get())) {
            if (currentCamera == nullptr) {
                currentCamera = camera;
                currentCamera->_setActive(true);
                //log("Using camera", currentCamera->toString());
            }
        } else if (auto* meshInstance = dynamic_cast<MeshInstance*>(node.get())) {
            if (meshInstance->isValid()) {
                const auto index = models.size();
                models.push_back(meshInstance);
                addingModel(meshInstance, index);
                descriptorSetNeedUpdate = true;
                createOrUpdateResources();
                addedModel(meshInstance);
            }
            } else if (auto* viewport = dynamic_cast<Viewport*>(node.get())) {
            currentViewport = viewport;
            //log("Using viewport", currentViewport->toString());
        }
    }

    void ModelsRenderer::removeNode(const shared_ptr<z0::Node> &node) {
        if (auto* camera = dynamic_cast<Camera*>(node.get())) {
            if (camera == currentCamera) {
                currentCamera->_setActive(false);
                currentCamera = nullptr;
            }
        } else if (auto* meshInstance = dynamic_cast<MeshInstance*>(node.get())) {
            auto it = find(models.begin(), models.end(), meshInstance);
            if (it != models.end()) {
                models.erase(it);
                removingModel(meshInstance);
            }
            descriptorSetNeedUpdate = true;
        } else if (auto* viewport = dynamic_cast<Viewport*>(node.get())) {
            if (currentViewport == viewport) {
                currentViewport = nullptr;
            }
        }
    }

    void ModelsRenderer::activateCamera(Camera* camera) {
        if (currentCamera != nullptr) currentCamera->_setActive(false);
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

    void ModelsRenderer::setInitialState(VkCommandBuffer commandBuffer) {
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
            setViewport(commandBuffer, getDevice().getSwapChainExtent().width, getDevice().getSwapChainExtent().height);
        }

        vkCmdSetRasterizationSamplesEXT(commandBuffer, device.getSamples());

        VkBool32 color_blend_enables[] = {VK_FALSE};
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

}