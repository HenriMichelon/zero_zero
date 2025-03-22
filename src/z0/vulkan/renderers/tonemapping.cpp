/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/vulkan.h"
#include "z0/libraries.h"

module z0.vulkan.TonemappingPostprocessingRenderer;

import z0.Application;
import z0.Tools;

import z0.vulkan.ColorFrameBufferHDR;
import z0.vulkan.DepthFrameBuffer;
import z0.vulkan.Descriptors;
import z0.vulkan.Device;
import z0.vulkan.PostprocessingRenderer;

namespace z0 {

    TonemappingPostprocessingRenderer::TonemappingPostprocessingRenderer(
            Device& device,
            const vector<shared_ptr<ColorFrameBufferHDR>>& inputColorAttachmentHdr,
            const vector<shared_ptr<DepthFrameBuffer>>& depthAttachement):
        PostprocessingRenderer{
            device, "reinhard", &globalUbo, sizeof(GlobalUniformBuffer),
            inputColorAttachmentHdr, depthAttachement, {}} {}

    void TonemappingPostprocessingRenderer::update(const uint32_t currentFrame) {
        const auto& config = app().getConfig();
        globalUbo.gamma = config.gamma;
        globalUbo.exposure = config.exposure;
        PostprocessingRenderer::update(currentFrame);
    }

}
