module;
#include "z0/libraries.h"

export module z0:TonemappingPostprocessingRenderer;

import :PostprocessingRenderer;
import :Device;
import :ColorFrameBufferHDR;
import :DepthFrameBuffer;

export namespace z0 {

    /**
     * Fragment shader based post processing renderer
     */
    class TonemappingPostprocessingRenderer : public PostprocessingRenderer {
    public:
        TonemappingPostprocessingRenderer(Device& device,
                                     const string& shaderDirectory,
                                     const vector<shared_ptr<ColorFrameBufferHDR>>& inputColorAttachmentHdr,
                                     const vector<shared_ptr<DepthFrameBuffer>>& depthBuffer);

        void loadShaders() override;

        void update(uint32_t currentFrame) override;

        void createDescriptorSetLayout() override;

        void createOrUpdateDescriptorSet(bool create) override;

    private:
        struct GlobalUniformBufferObject {
            alignas(4) float gamma{2.2};
            alignas(4) float exposure{1.0};
        };

        vector<shared_ptr<DepthFrameBuffer>> resolvedDepthBuffer;
    };
}
