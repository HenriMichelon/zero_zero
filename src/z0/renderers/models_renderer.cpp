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

    void ModelsRenderer::addNode(const shared_ptr<Node> &node, const uint32_t currentFrame) {
        if (auto *camera = dynamic_cast<Camera *>(node.get())) {
            if (frameData[currentFrame].currentCamera == nullptr) {
                activateCamera(camera, currentFrame);
                //log("Using camera", currentCamera->toString());
            }
        } else if (auto *meshInstance = dynamic_cast<MeshInstance *>(node.get())) {
            if (meshInstance->isValid()) {
                if (meshInstance->getMesh()->_getMaterials().empty())
                    die("Models without materials are not supported");
                const auto index = frameData[currentFrame].models.size();
                frameData[currentFrame].models.push_back(meshInstance);
                addingModel(meshInstance, index, currentFrame);
                descriptorSetNeedUpdate = true;
                createOrUpdateResources();
                addedModel(meshInstance, currentFrame);
            }
        } else if (auto *viewport = dynamic_cast<Viewport *>(node.get())) {
            frameData[currentFrame].currentViewport = viewport;
            //log("Using viewport", currentViewport->toString());
        }
    }

    void ModelsRenderer::removeNode(const shared_ptr<Node> &node, const uint32_t currentFrame) {
        if (const auto *camera = dynamic_cast<Camera *>(node.get())) {
            if (camera == frameData[currentFrame].currentCamera) {
                frameData[currentFrame].currentCamera->_setActive(false);
                frameData[currentFrame].currentCamera = nullptr;
            }
        } else if (auto *meshInstance = dynamic_cast<MeshInstance *>(node.get())) {
            const auto it = find(frameData[currentFrame].models.begin(), frameData[currentFrame].models.end(), meshInstance);
            if (it != frameData[currentFrame].models.end()) {
                frameData[currentFrame].models.erase(it);
                removingModel(meshInstance, currentFrame);
            }
            descriptorSetNeedUpdate = true;
        } else if (const auto *viewport = dynamic_cast<Viewport *>(node.get())) {
            if (frameData[currentFrame].currentViewport == viewport) {
                frameData[currentFrame].currentViewport = nullptr;
            }
        }
    }

    void ModelsRenderer::activateCamera(Camera *camera, const uint32_t currentFrame) {
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
        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            frameData[i].depthFrameBuffer->cleanupImagesResources();
            ModelsRenderer::frameData[i].modelUniformBuffer.reset();
        }
        cleanupImagesResources();
        Renderpass::cleanup();
    }

    ModelsRenderer::ModelsRenderer(Device &device, const string &shaderDirectory, vec3 clearColor):
        Renderpass(device, shaderDirectory, clearColor) {
    }

    // Set the initial states of the dynamic rendering
    void ModelsRenderer::setInitialState(const VkCommandBuffer commandBuffer, const uint32_t currentFrame) const {
        bindShaders(commandBuffer);
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
