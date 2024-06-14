#pragma once

namespace z0 {

    class SimplePostprocessingRenderer: public BasePostprocessingRenderer {
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

}