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
            device, "reinhard",
            inputColorAttachmentHdr, depthAttachement, {}} {}

    void TonemappingPostprocessingRenderer::update(const uint32_t currentFrame) {
        const auto& config = app().getConfig();
        const GlobalUniformBuffer globalUbo {
            .gamma = config.gamma,
            .exposure = config.exposure,
        };
        writeUniformBuffer(globalBuffer[currentFrame], &globalUbo);
    }

    void TonemappingPostprocessingRenderer::createDescriptorSetLayout() {
        globalBufferSize = sizeof(GlobalUniformBuffer);
        PostprocessingRenderer::createDescriptorSetLayout();
    }

}
