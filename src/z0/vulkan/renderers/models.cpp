/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/vulkan.h"
#include "z0/libraries.h"

module z0.vulkan.ModelsRenderer;

import z0.Tools;

import z0.nodes.Camera;
import z0.nodes.MeshInstance;
import z0.nodes.Node;
import z0.nodes.Viewport;

import z0.resources.Mesh;

import z0.vulkan.Buffer;
import z0.vulkan.Device;
import z0.vulkan.DepthFrameBuffer;
import z0.vulkan.Renderer;
import z0.vulkan.Renderpass;
import z0.vulkan.Mesh;

 namespace z0 {

    void ModelsRenderer::addNode(const shared_ptr<Node> &node, const uint32_t currentFrame) {
        auto& frame = frameData[currentFrame];
        if (const auto& camera = dynamic_pointer_cast<Camera>(node)) {
            if (frame.currentCamera == nullptr) {
                activateCamera(camera, currentFrame);
                //log("Using camera", currentCamera->toString());
            }
        } else if (const auto& meshInstance = dynamic_pointer_cast<MeshInstance>(node)) {
            if (meshInstance->isValid()) {
                if (meshInstance->getMesh()->_getMaterials().empty()) {
                    die("Models without materials are not supported");
                }
                frame.models.push_back(meshInstance);
                addingModel(meshInstance, currentFrame);
                descriptorSetNeedUpdate = true;
                createOrUpdateResources();
                frame.modelsDirty = true;
            }
        } else if (const auto& viewport = dynamic_pointer_cast<Viewport>(node)) {
            frameData[currentFrame].currentViewport = viewport;
            //log("Using viewport", currentViewport->toString());
        }
    }

    void ModelsRenderer::removeNode(const shared_ptr<Node> &node, const uint32_t currentFrame) {
        auto& frame = frameData[currentFrame];
        if (const auto& camera = dynamic_pointer_cast<Camera>(node)) {
            if (camera == frame.currentCamera) {
                frame.currentCamera->_setActive(false);
                frame.currentCamera = nullptr;
            }
        } else if (const auto& meshInstance = dynamic_pointer_cast<MeshInstance>(node)) {
            const auto it = ranges::find(frame.models, meshInstance);
            if (it != frame.models.end()) {
                frame.models.erase(it);
                removingModel(meshInstance, currentFrame);
                frame.modelsDirty = true;
            }
            descriptorSetNeedUpdate = true;
        } else if (const auto& viewport = dynamic_pointer_cast<Viewport>(node)) {
            if (frame.currentViewport == viewport) {
                frame.currentViewport = nullptr;
            }
        }
    }

    void ModelsRenderer::activateCamera(const shared_ptr<Camera> &camera, const uint32_t currentFrame) {
        if (frameData[currentFrame].currentCamera != nullptr)
            frameData[currentFrame].currentCamera->_setActive(false);
        if (camera == nullptr) {
            frameData[currentFrame].currentCamera = nullptr;
        } else {
            frameData[currentFrame].currentCamera = camera;
            frameData[currentFrame].currentCamera->_setActive(true);
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
        Renderer{false},
        Renderpass(device, clearColor) {
        frameData.resize(device.getFramesInFlight());
    }

    // Set the initial states of the dynamic rendering
    void ModelsRenderer::setInitialState(const VkCommandBuffer commandBuffer, const uint32_t currentFrame, const bool loadShaders) const {
        if (loadShaders) { bindShaders(commandBuffer); }
        if (frameData[currentFrame].currentViewport != nullptr) {
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
                            static_cast<int32_t>(frameData[currentFrame].currentViewport->getViewportPosition().x),
                            static_cast<int32_t>(frameData[currentFrame].currentViewport->getViewportPosition().y)
                    },
                    .extent = {
                            static_cast<uint32_t>(frameData[currentFrame].currentViewport->getViewportSize().x),
                            static_cast<uint32_t>(frameData[currentFrame].currentViewport->getViewportSize().y)
                    },
            };
            vkCmdSetScissorWithCount(commandBuffer, 1, &scissor);
        } else {
            setViewport(commandBuffer,
                        getDevice().getSwapChainExtent().width,
                        getDevice().getSwapChainExtent().height);
        }

        vkCmdSetRasterizationSamplesEXT(commandBuffer, device.getSamples());


        const auto vertexBinding   = VulkanMesh::getBindingDescription();
        const auto vertexAttribute = VulkanMesh::getAttributeDescription();
        vkCmdSetVertexInputEXT(commandBuffer,
                               vertexBinding.size(),
                               vertexBinding.data(),
                               vertexAttribute.size(),
                               vertexAttribute.data());
    }

}
