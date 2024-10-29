module;
#include "z0/libraries.h"

export module z0:TonemappingPostprocessingRenderer;

import :PostprocessingRenderer;
import :Device;
import :Buffer;
import :ColorFrameBufferHDR;
import :DepthFrameBuffer;

export namespace z0 {

    /*
     * Fragment shader based post processing renderer
     */
    class TonemappingPostprocessingRenderer : public PostprocessingRenderer {
    public:
        TonemappingPostprocessingRenderer(Device& device,
                                     const vector<shared_ptr<ColorFrameBufferHDR>>& inputColorAttachmentHdr,
                                     const vector<shared_ptr<DepthFrameBuffer>>& depthBuffer);

        void loadShaders() override;

        void cleanup() override;

        void update(uint32_t currentFrame) override;

        void createDescriptorSetLayout() override;

        void createOrUpdateDescriptorSet(bool create) override;

    private:
        struct GlobalUniformBuffer {
            alignas(4) float gamma{2.2};
            alignas(4) float exposure{1.0};
        };

        vector<unique_ptr<Buffer>>           globalBuffer;
        vector<shared_ptr<DepthFrameBuffer>> resolvedDepthBuffer;
    };
}
