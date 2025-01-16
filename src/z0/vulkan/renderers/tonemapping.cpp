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
            Device &            device,
            const vector<shared_ptr<ColorFrameBufferHDR>>&inputColorAttachmentHdr,
            const vector<shared_ptr<DepthFrameBuffer>>& depthBuffer):
        PostprocessingRenderer{device, inputColorAttachmentHdr},
        resolvedDepthBuffer{depthBuffer} {
        globalBuffer.resize(device.getFramesInFlight());
        createOrUpdateResources();
    }

    void TonemappingPostprocessingRenderer::cleanup() {
        globalBuffer.clear();
        PostprocessingRenderer::cleanup();
    }

    void TonemappingPostprocessingRenderer::update(const uint32_t currentFrame) {
        const GlobalUniformBuffer globalUbo {
            .gamma = app().getConfig().gamma,
            .exposure = app().getConfig().exposure,
        };
        writeUniformBuffer(globalBuffer.at(currentFrame), &globalUbo);
    }

    void TonemappingPostprocessingRenderer::createDescriptorSetLayout() {
        descriptorPool = DescriptorPool::Builder(device)
                                 .setMaxSets(device.getFramesInFlight())
                                 .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, device.getFramesInFlight())
                                 .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, device.getFramesInFlight())
                                 .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, device.getFramesInFlight())
                                 .build();
        setLayout = DescriptorSetLayout::Builder(device)
                            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
                            // source color attachment
                            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
                            // source depth attachment
                            .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
                            .build();
        for (auto i = 0; i < device.getFramesInFlight(); i++) {
            globalBuffer.at(i) = createUniformBuffer(sizeof(GlobalUniformBuffer));
        }
    }

    void TonemappingPostprocessingRenderer::createOrUpdateDescriptorSet(const bool create) {
        for (auto i = 0; i < device.getFramesInFlight(); i++) {
            auto globalBufferInfo = globalBuffer.at(i)->descriptorInfo(sizeof(GlobalUniformBuffer));
            auto colorInfo        = inputColorAttachmentHdr.at(i)->imageInfo();
            auto depthInfo        = VkDescriptorImageInfo {
                .sampler = colorInfo.sampler,
                .imageView = resolvedDepthBuffer.at(i)->getImageView(),
                .imageLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
            };
            auto writer = DescriptorWriter(*setLayout, *descriptorPool)
                .writeBuffer(0, &globalBufferInfo)
                .writeImage(1, &colorInfo)
                .writeImage(2, &depthInfo);
            if (!writer.build(descriptorSet.at(i), create))
                die("Cannot allocate descriptor set for TonemappingPostprocessingRenderer");
        }
    }

    void TonemappingPostprocessingRenderer::loadShaders() {
        PostprocessingRenderer::loadShaders();
        fragShader = createShader("reinhard.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

}
