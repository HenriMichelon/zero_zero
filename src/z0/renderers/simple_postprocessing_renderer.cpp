#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/renderers/base_renderpass.h"
#include "z0/renderers/base_renderer.h"
#include "z0/renderers/base_postprocessing_renderer.h"
#include "z0/renderers/simple_postprocessing_renderer.h"
#endif

namespace z0 {

    SimplePostprocessingRenderer::SimplePostprocessingRenderer(Device &dev,
                                                               string shaderDirectory,
                                                               const string _shaderName,
                                                               SampledFrameBuffer* inputColorAttachmentHdr):
            BasePostprocessingRenderer{dev, shaderDirectory, inputColorAttachmentHdr}, shaderName{_shaderName} {
        createOrUpdateResources();
    }

    void SimplePostprocessingRenderer::loadShaders() {
        BasePostprocessingRenderer::loadShaders();
        fragShader = createShader(shaderName + ".frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void SimplePostprocessingRenderer::update(uint32_t currentFrame) {
        GobalUniformBufferObject globalUbo {
        };
        writeUniformBuffer(globalUniformBuffers, currentFrame, &globalUbo);
    }

    void SimplePostprocessingRenderer::createDescriptorSetLayout() {
        globalUniformBufferSize = sizeof(GobalUniformBufferObject);
        BasePostprocessingRenderer::createDescriptorSetLayout();
    }

}