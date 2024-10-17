module;
#include "z0/libraries.h"

export module z0:SimplePostprocessingRenderer;

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
                                     const string& shaderName,
                                     const vector<SampledFrameBuffer*>& inputColorAttachmentHdr);

        void update( uint32_t currentFrame) override ;

        void loadShaders() override;

        void createDescriptorSetLayout() override;

    private:
        struct GobalUniformBufferObject {
            alignas(4) float dummy;
        };

        const string shaderName;
    };
}
