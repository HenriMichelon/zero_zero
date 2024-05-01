#include "z0/renderers/base_models_renderer.h"
#include "z0/resources/mesh.h"
#include "z0/descriptors.h"

namespace z0 {

    BaseModelsRenderer::BaseModelsRenderer(const Device &dev, std::string sDir):
        BaseRenderpass(dev, std::move(sDir)) {}

    void BaseModelsRenderer::addNode(const shared_ptr<Node>& node) {
        if (auto* camera = dynamic_cast<Camera*>(node.get())) {
            currentCamera = camera;
            log << "Using camera " << *currentCamera << endl;
        } else if (auto* meshInstance = dynamic_cast<MeshInstance*>(node.get())) {
            models.push_back(meshInstance);
            addingModel(meshInstance);
            log << "Added model " << *meshInstance << endl;
        }
        createResources();
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
        auto vertexBinding = Mesh::_getBindingDescription();
        auto vertexAttribute = Mesh::_getAttributeDescription();
        vkCmdSetVertexInputEXT(commandBuffer,
                               vertexBinding.size(),
                               vertexBinding.data(),
                               vertexAttribute.size(),
                               vertexAttribute.data());
    }

}