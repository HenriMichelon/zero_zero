module;
#include "z0/libraries.h"

export module z0:SimplePostprocessingRenderer;

import :PostprocessingRenderer;
import :Device;
import :ColorFrameBufferHDR;

export namespace z0 {

    /*
     * Fragment shader based post processing renderer
     */
    class SimplePostprocessingRenderer : public PostprocessingRenderer {
    public:
        SimplePostprocessingRenderer(Device& device,
                                     const string& shaderName,
                                     const vector<shared_ptr<ColorFrameBufferHDR>>& inputColorAttachmentHdr);

        void loadShaders() override;

    private:
        const string shaderName;
    };
}
