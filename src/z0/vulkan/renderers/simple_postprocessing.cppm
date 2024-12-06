/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.vulkan.SimplePostprocessingRenderer;

import z0.vulkan.ColorFrameBufferHDR;
import z0.vulkan.Device;
import z0.vulkan.PostprocessingRenderer;

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
