module;
#include "z0/libraries.h"

export module Z0:SimplePostprocessingRenderer;

import :PostprocessingRenderer;
import :Device;
import :SampledFrameBuffer;

export namespace z0 {

    /**
     * Fragment shader based post processing renderer
     */
    class SimplePostprocessingRenderer : public PostprocessingRenderer {
    public:
        SimplePostprocessingRenderer(Device& device,
                                     const string& shaderDirectory,
                                     const string& _shaderName,
                                     SampledFrameBuffer* inputColorAttachmentHdr):
            PostprocessingRenderer{device, shaderDirectory, inputColorAttachmentHdr}, shaderName{_shaderName} {
            createOrUpdateResources();
        }

        void update(const uint32_t currentFrame) override {
            constexpr GobalUniformBufferObject globalUbo{};
            writeUniformBuffer(globalUniformBuffers, currentFrame, &globalUbo);
        }

        void loadShaders() override {
            PostprocessingRenderer::loadShaders();
            fragShader = createShader(shaderName + ".frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
        }

        void createDescriptorSetLayout() override {
            globalUniformBufferSize = sizeof(GobalUniformBufferObject);
            PostprocessingRenderer::createDescriptorSetLayout();
        }

    private:
        struct GobalUniformBufferObject {
            alignas(4) float dummy;
        };

        const string shaderName;
    };
}
