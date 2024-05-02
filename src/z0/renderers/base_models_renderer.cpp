#include "z0/renderers/base_models_renderer.h"
#include "z0/resources/mesh.h"
#include "z0/descriptors.h"

#include <algorithm>

namespace z0 {

    BaseModelsRenderer::BaseModelsRenderer(const Device &dev, const string& sDir):
        BaseRenderpass(dev, sDir) {}

    void BaseModelsRenderer::addNode(const shared_ptr<Node>& node) {
        if (auto* camera = dynamic_cast<Camera*>(node.get())) {
            if (currentCamera == nullptr) {
                currentCamera = camera;
                currentCamera->_setActive(true);
                //log << "Using camera " << *currentCamera << endl;
            }
        } else if (auto* meshInstance = dynamic_cast<MeshInstance*>(node.get())) {
            if (meshInstance->isValid()) {
                const auto index = models.size();
                models.push_back(meshInstance);
                addingModel(meshInstance, index);
                descriptorSetNeedUpdate = true;
                createOrUpdateResources();
                addedModel(meshInstance);
                //log << "Added model " << *meshInstance << endl;
            }
        }
    }

    void BaseModelsRenderer::removeNode(const shared_ptr<z0::Node> &node) {
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
            //log << "Removed model " << *meshInstance << endl;
        }
    }

    void BaseModelsRenderer::activateCamera(const shared_ptr<z0::Camera> &camera) {
        if (currentCamera != nullptr) currentCamera->_setActive(false);
        currentCamera = camera.get();
        currentCamera->_setActive(true);
        //log << "Using camera " << *currentCamera << endl;
    }

    void BaseModelsRenderer::cleanup() {
        depthFrameBuffer->cleanupImagesResources();
        cleanupImagesResources();
        modelUniformBuffers.clear();
        BaseRenderpass::cleanup();
    }

    void BaseModelsRenderer::setInitialState(VkCommandBuffer commandBuffer) {
        bindShaders(commandBuffer);
        vkCmdSetRasterizationSamplesEXT(commandBuffer, device.getSamples());
        vkCmdSetDepthTestEnable(commandBuffer, VK_TRUE);
        setViewport(commandBuffer, device.getSwapChainExtent().width, device.getSwapChainExtent().height);
        const auto vertexBinding = Mesh::_getBindingDescription();
        const auto vertexAttribute = Mesh::_getAttributeDescription();
        vkCmdSetVertexInputEXT(commandBuffer,
                               vertexBinding.size(),
                               vertexBinding.data(),
                               vertexAttribute.size(),
                               vertexAttribute.data());
    }

}