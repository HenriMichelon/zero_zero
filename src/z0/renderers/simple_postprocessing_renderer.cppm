module;
#include "z0/modules.h"

export module Z0:SimplePostprocessingRenderer;

import :PostprocessingRenderer;
import :Device;
import :SampledFrameBuffer;

export namespace z0 {

    /**
     * Fragment shader based post processing renderer
     */
    class SimplePostprocessingRenderer: public PostprocessingRenderer {
    public:
        SimplePostprocessingRenderer(Device& device,
                                     string shaderDirectory,
                                     const string shaderName,
                                     SampledFrameBuffer* inputColorAttachmentHdr);

        void update(uint32_t currentFrame) override;
        void loadShaders() override;
        void createDescriptorSetLayout()  override;

    private:
        struct GobalUniformBufferObject {
            alignas(4) float dummy;
        };
        const string shaderName;
    };

    SimplePostprocessingRenderer::SimplePostprocessingRenderer(Device &dev,
                                                              string shaderDirectory,
                                                              const string _shaderName,
                                                              SampledFrameBuffer* inputColorAttachmentHdr):
           PostprocessingRenderer{dev, shaderDirectory, inputColorAttachmentHdr}, shaderName{_shaderName} {
        createOrUpdateResources();
    }

    void SimplePostprocessingRenderer::loadShaders() {
        PostprocessingRenderer::loadShaders();
        fragShader = createShader(shaderName + ".frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void SimplePostprocessingRenderer::update(uint32_t currentFrame) {
        GobalUniformBufferObject globalUbo {
        };
        writeUniformBuffer(globalUniformBuffers, currentFrame, &globalUbo);
    }

    void SimplePostprocessingRenderer::createDescriptorSetLayout() {
        globalUniformBufferSize = sizeof(GobalUniformBufferObject);
        PostprocessingRenderer::createDescriptorSetLayout();
    }

}