/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>
#include "z0/libraries.h"

module z0.ModelsRenderer;

import z0.Camera;
import z0.Mesh;
import z0.MeshInstance;
import z0.Node;
import z0.Tools;
import z0.Viewport;

import z0.Buffer;
import z0.Device;
import z0.DepthFrameBuffer;
import z0.Renderer;
import z0.Renderpass;
import z0.VulkanMesh;

 namespace z0 {

    void ModelsRenderer::addNode(const shared_ptr<Node> &node, const uint32_t currentFrame) {
        if (const auto& camera = dynamic_pointer_cast<Camera>(node)) {
            if (frameData.at(currentFrame).currentCamera == nullptr) {
                activateCamera(camera, currentFrame);
                //log("Using camera", currentCamera->toString());
            }
        } else if (const auto& meshInstance = dynamic_pointer_cast<MeshInstance>(node)) {
            if (meshInstance->isValid()) {
                if (meshInstance->getMesh()->_getMaterials().empty()) {
                    die("Models without materials are not supported");
                }
                const auto index = frameData.at(currentFrame).models.size();
                frameData.at(currentFrame).models.push_back(meshInstance);
                addingModel(meshInstance, index, currentFrame);
                descriptorSetNeedUpdate = true;
                createOrUpdateResources();
                addedModel(meshInstance, currentFrame);
            }
        } else if (const auto& viewport = dynamic_pointer_cast<Viewport>(node)) {
            frameData.at(currentFrame).currentViewport = viewport;
            //log("Using viewport", currentViewport->toString());
        }
    }

    void ModelsRenderer::removeNode(const shared_ptr<Node> &node, const uint32_t currentFrame) {
        if (const auto& camera = dynamic_pointer_cast<Camera>(node)) {
            if (camera == frameData.at(currentFrame).currentCamera) {
                frameData.at(currentFrame).currentCamera->_setActive(false);
                frameData.at(currentFrame).currentCamera = nullptr;
            }
        } else if (const auto& meshInstance = dynamic_pointer_cast<MeshInstance>(node)) {
            const auto it = ranges::find(frameData.at(currentFrame).models, meshInstance);
            if (it != frameData.at(currentFrame).models.end()) {
                frameData.at(currentFrame).models.erase(it);
                removingModel(meshInstance, currentFrame);
            }
            descriptorSetNeedUpdate = true;
        } else if (const auto& viewport = dynamic_pointer_cast<Viewport>(node)) {
            if (frameData.at(currentFrame).currentViewport == viewport) {
                frameData.at(currentFrame).currentViewport = nullptr;
            }
        }
    }

    void ModelsRenderer::activateCamera(const shared_ptr<Camera> &camera, const uint32_t currentFrame) {
        if (frameData.at(currentFrame).currentCamera != nullptr)
            frameData.at(currentFrame).currentCamera->_setActive(false);
        if (camera == nullptr) {
            frameData.at(currentFrame).currentCamera = nullptr;
        } else {
            frameData.at(currentFrame).currentCamera = camera;
            frameData.at(currentFrame).currentCamera->_setActive(true);
        }
    }

    void ModelsRenderer::cleanup() {
        for(auto& frame: frameData) {
            frame.depthFrameBuffer->cleanupImagesResources();
            frame.modelUniformBuffer.reset();
        }
        cleanupImagesResources();
        Renderpass::cleanup();
    }

    ModelsRenderer::ModelsRenderer(Device &device, const vec3 clearColor):
        Renderpass(device, clearColor) {
        frameData.resize(device.getFramesInFlight());
    }

    // Set the initial states of the dynamic rendering
    void ModelsRenderer::setInitialState(const VkCommandBuffer commandBuffer, const uint32_t currentFrame, const bool loadShaders) const {
        if (loadShaders) { bindShaders(commandBuffer); }
        if (frameData.at(currentFrame).currentViewport != nullptr) {
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
                            static_cast<int32_t>(frameData.at(currentFrame).currentViewport->getViewportPosition().x),
                            static_cast<int32_t>(frameData.at(currentFrame).currentViewport->getViewportPosition().y)
                    },
                    .extent = {
                            static_cast<uint32_t>(frameData.at(currentFrame).currentViewport->getViewportSize().x),
                            static_cast<uint32_t>(frameData.at(currentFrame).currentViewport->getViewportSize().y)
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

        const auto vertexBinding   = VulkanMesh::getBindingDescription();
        const auto vertexAttribute = VulkanMesh::getAttributeDescription();
        vkCmdSetVertexInputEXT(commandBuffer,
                               vertexBinding.size(),
                               vertexBinding.data(),
                               vertexAttribute.size(),
                               vertexAttribute.data());
    }

}
