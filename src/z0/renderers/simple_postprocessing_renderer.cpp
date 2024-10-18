module;
#include "z0/libraries.h"

module z0;

import :PostprocessingRenderer;
import :Device;
import :ColorFrameBufferHDR;
import :SimplePostprocessingRenderer;

namespace z0 {

    SimplePostprocessingRenderer::SimplePostprocessingRenderer(Device &            device,
                                                               const string &      shaderDirectory,
                                                               const string &      shaderName,
                                                                const vector<ColorFrameBufferHDR*>&inputColorAttachmentHdr):
        PostprocessingRenderer{device, shaderDirectory, inputColorAttachmentHdr}, shaderName{shaderName} {
        createOrUpdateResources();
    }

    void SimplePostprocessingRenderer::update(const uint32_t currentFrame) {
        constexpr GobalUniformBufferObject globalUbo{};
        writeUniformBuffer(globalUniformBuffers[currentFrame], &globalUbo);
    }

    void SimplePostprocessingRenderer::loadShaders() {
        PostprocessingRenderer::loadShaders();
        fragShader = createShader(shaderName + ".frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void SimplePostprocessingRenderer::createDescriptorSetLayout() {
        globalUniformBufferSize = sizeof(GobalUniformBufferObject);
        PostprocessingRenderer::createDescriptorSetLayout();
    }

}
