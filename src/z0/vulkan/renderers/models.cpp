module;
#include <volk.h>
#include "z0/libraries.h"

module z0;

import :Node;
import :Camera;
import :MeshInstance;
import :Viewport;
import :Mesh;

import :Renderer;
import :Renderpass;
import :Buffer;
import :Device;
import :DepthFrameBuffer;
import :ModelsRenderer;
import :VulkanMesh;

 namespace z0 {

    void ModelsRenderer::addNode(const shared_ptr<Node> &node, const uint32_t currentFrame) {
        if (const auto& camera = dynamic_pointer_cast<Camera>(node)) {
            if (frameData[currentFrame].currentCamera == nullptr) {
                activateCamera(camera, currentFrame);
                //log("Using camera", currentCamera->toString());
            }
        } else if (const auto& meshInstance = dynamic_pointer_cast<MeshInstance>(node)) {
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
        } else if (const auto& viewport = dynamic_pointer_cast<Viewport>(node)) {
            frameData[currentFrame].currentViewport = viewport;
            //log("Using viewport", currentViewport->toString());
        }
    }

    void ModelsRenderer::removeNode(const shared_ptr<Node> &node, const uint32_t currentFrame) {
        if (const auto& camera = dynamic_pointer_cast<Camera>(node)) {
            if (camera == frameData[currentFrame].currentCamera) {
                frameData[currentFrame].currentCamera->_setActive(false);
                frameData[currentFrame].currentCamera = nullptr;
            }
        } else if (const auto& meshInstance = dynamic_pointer_cast<MeshInstance>(node)) {
            const auto it = find(frameData[currentFrame].models.begin(), frameData[currentFrame].models.end(), meshInstance);
            if (it != frameData[currentFrame].models.end()) {
                frameData[currentFrame].models.erase(it);
                removingModel(meshInstance, currentFrame);
            }
            descriptorSetNeedUpdate = true;
        } else if (const auto& viewport = dynamic_pointer_cast<Viewport>(node)) {
            if (frameData[currentFrame].currentViewport == viewport) {
                frameData[currentFrame].currentViewport = nullptr;
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

    ModelsRenderer::ModelsRenderer(Device &device, const string &shaderDirectory, vec3 clearColor):
        Renderpass(device, shaderDirectory, clearColor) {
        frameData.resize(device.getFramesInFlight());
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

        const auto vertexBinding   = VulkanMesh::getBindingDescription();
        const auto vertexAttribute = VulkanMesh::getAttributeDescription();
        vkCmdSetVertexInputEXT(commandBuffer,
                               vertexBinding.size(),
                               vertexBinding.data(),
                               vertexAttribute.size(),
                               vertexAttribute.data());
    }

}
