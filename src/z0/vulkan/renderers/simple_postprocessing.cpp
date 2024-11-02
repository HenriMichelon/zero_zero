/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

module z0;

import :PostprocessingRenderer;
import :Device;
import :ColorFrameBufferHDR;
import :SimplePostprocessingRenderer;

namespace z0 {

    SimplePostprocessingRenderer::SimplePostprocessingRenderer(Device &            device,
                                                               const string &      shaderName,
                                                                const vector<shared_ptr<ColorFrameBufferHDR>>&inputColorAttachmentHdr):
        PostprocessingRenderer{device, inputColorAttachmentHdr}, shaderName{shaderName} {
        createOrUpdateResources();
    }

    void SimplePostprocessingRenderer::loadShaders() {
        PostprocessingRenderer::loadShaders();
        fragShader = createShader(shaderName + ".frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

}
