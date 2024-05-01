#include "z0/renderers/base_models_renderer.h"
#include "z0/resources/mesh.h"
#include "z0/descriptors.h"

namespace z0 {

    BaseModelsRenderer::BaseModelsRenderer(const Device &dev, std::string sDir):
        BaseRenderpass(dev, sDir) {}

    void BaseModelsRenderer::cleanup() {
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