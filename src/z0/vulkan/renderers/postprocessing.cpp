/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/vulkan.h"
#include "z0/libraries.h"

module z0.vulkan.PostprocessingRenderer;

import z0.Constants;
import z0.Tools;

import z0.vulkan.ColorFrameBufferHDR;
import z0.vulkan.Descriptors;
import z0.vulkan.Device;
import z0.vulkan.Renderer;
import z0.vulkan.Renderpass;

namespace z0 {

    PostprocessingRenderer::PostprocessingRenderer(Device &device,
                                                   const vector<shared_ptr<ColorFrameBufferHDR>> & inputColorAttachment) :
        Renderer{false},
        Renderpass{device, WINDOW_CLEAR_COLOR}, inputColorAttachmentHdr{inputColorAttachment} {
        colorAttachmentHdr.resize(device.getFramesInFlight());
        inputColorAttachmentHdr.resize(device.getFramesInFlight());
        createImagesResources();
    }

    void PostprocessingRenderer::setInputColorAttachments(const vector<shared_ptr<ColorFrameBufferHDR>> &input) {
        inputColorAttachmentHdr = input;
        createOrUpdateDescriptorSet(false);
    }


    void PostprocessingRenderer::cleanup() {
        cleanupImagesResources();
        Renderpass::cleanup();
    }

    void PostprocessingRenderer::loadShaders() {
        vertShader = createShader("quad.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
    }

    void PostprocessingRenderer::drawFrame(const uint32_t currentFrame, const bool isLast) {
        beginRendering(currentFrame);
        const auto& commandBuffer = commandBuffers[currentFrame];
        bindShaders(commandBuffer);
        vkCmdSetRasterizationSamplesEXT(commandBuffer, VK_SAMPLE_COUNT_1_BIT);
        vkCmdSetDepthTestEnable(commandBuffer, VK_FALSE);
        setViewport(commandBuffer, device.getSwapChainExtent().width, device.getSwapChainExtent().height);
        vkCmdSetVertexInputEXT(commandBuffer, 0, nullptr, 0, nullptr);
        vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_NONE);
        bindDescriptorSets(commandBuffer, currentFrame);
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
        endRendering(currentFrame, isLast);
    }

    void PostprocessingRenderer::createDescriptorSetLayout() {
        descriptorPool = DescriptorPool::Builder(device)
                                 .setMaxSets(device.getFramesInFlight())
                                 .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, device.getFramesInFlight())
                                 .build();
        setLayout = DescriptorSetLayout::Builder(device)
                            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
                            .build();
    }

    void PostprocessingRenderer::createOrUpdateDescriptorSet(const bool create) {
        for (auto i = 0; i < device.getFramesInFlight(); i++) {
            auto imageInfo = inputColorAttachmentHdr[i]->imageInfo();
            auto writer    = DescriptorWriter(*setLayout, *descriptorPool)
                .writeImage(0, &imageInfo);
            if (!writer.build(descriptorSet[i], create))
                die("Cannot allocate descriptor set for PostprocessingRenderer");
        }
    }

    void PostprocessingRenderer::createImagesResources() {
        for (auto i = 0; i < device.getFramesInFlight(); i++) {
            colorAttachmentHdr[i] = make_shared<ColorFrameBufferHDR>(device);
        }
    }

    void PostprocessingRenderer::cleanupImagesResources() {
        for (auto i = 0; i < device.getFramesInFlight(); i++) {
            colorAttachmentHdr[i]->cleanupImagesResources();
        }
    }

    void PostprocessingRenderer::recreateImagesResources() {
        for (auto i = 0; i < device.getFramesInFlight(); i++) {
            colorAttachmentHdr[i]->cleanupImagesResources();
            colorAttachmentHdr[i]->createImagesResources();
        }
    }

    void PostprocessingRenderer::beginRendering(const uint32_t currentFrame) {
        const auto& commandBuffer = commandBuffers[currentFrame];
        device.transitionImageLayout(commandBuffer,
                                     colorAttachmentHdr[currentFrame]->getImage(),
                                     VK_IMAGE_LAYOUT_UNDEFINED,
                                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                     0,
                                     VK_ACCESS_TRANSFER_WRITE_BIT,
                                     VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                     VK_PIPELINE_STAGE_TRANSFER_BIT,
                                     VK_IMAGE_ASPECT_COLOR_BIT);
        const VkRenderingAttachmentInfo colorAttachmentInfo{.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                                                            .imageView   = colorAttachmentHdr[currentFrame]->getImageView(),
                                                            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                            .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                            .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
                                                            .clearValue  = clearColor};
        const VkRenderingInfo           renderingInfo{.sType                = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
                                                      .pNext                = nullptr,
                                                      .renderArea           = {{0, 0}, device.getSwapChainExtent()},
                                                      .layerCount           = 1,
                                                      .colorAttachmentCount = 1,
                                                      .pColorAttachments    = &colorAttachmentInfo,
                                                      .pDepthAttachment     = nullptr,
                                                      .pStencilAttachment   = nullptr};
        vkCmdBeginRendering(commandBuffer, &renderingInfo);
    }

    void PostprocessingRenderer::endRendering(const uint32_t currentFrame, const bool isLast) {
        const auto& commandBuffer = commandBuffers[currentFrame];
        vkCmdEndRendering(commandBuffer);
        device.transitionImageLayout(commandBuffer,
                                     colorAttachmentHdr[currentFrame]->getImage(),
                                     VK_IMAGE_LAYOUT_UNDEFINED,
                                     isLast ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
                                            : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                     0,
                                     isLast ? VK_ACCESS_TRANSFER_READ_BIT : VK_ACCESS_SHADER_READ_BIT,
                                     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                     isLast ? VK_PIPELINE_STAGE_TRANSFER_BIT : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                     VK_IMAGE_ASPECT_COLOR_BIT);
    }

} // namespace z0
