module;
#include <volk.h>
#include "z0/libraries.h"

module z0;

import :Constants;
import :Tools;
import :Renderer;
import :Renderpass;
import :Device;
import :SampledFrameBuffer;
import :ColorFrameBufferHDR;
import :Descriptors;
import :PostprocessingRenderer;

namespace z0 {

    PostprocessingRenderer::PostprocessingRenderer(Device &device, const string &shaderDirectory,
                                                   const vector<SampledFrameBuffer *> & inputColorAttachment) :
        Renderpass{device, shaderDirectory, WINDOW_CLEAR_COLOR}, inputColorAttachmentHdr{inputColorAttachment} {
        createImagesResources();
    }

    void PostprocessingRenderer::setInputColorAttachments(const vector<SampledFrameBuffer *> &input) {
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

    void PostprocessingRenderer::recordCommands(const VkCommandBuffer commandBuffer, const uint32_t currentFrame) {
        bindShaders(commandBuffer);
        vkCmdSetRasterizationSamplesEXT(commandBuffer, VK_SAMPLE_COUNT_1_BIT);
        vkCmdSetDepthTestEnable(commandBuffer, VK_FALSE);
        setViewport(commandBuffer, device.getSwapChainExtent().width, device.getSwapChainExtent().height);
        vkCmdSetVertexInputEXT(commandBuffer, 0, nullptr, 0, nullptr);
        vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_NONE);
        bindDescriptorSets(commandBuffer, currentFrame);
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    }

    void PostprocessingRenderer::createDescriptorSetLayout() {
        descriptorPool = DescriptorPool::Builder(device)
                                 .setMaxSets(MAX_FRAMES_IN_FLIGHT)
                                 .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT)
                                 .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT)
                                 .build();
        setLayout = DescriptorSetLayout::Builder(device)
                            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
                            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
                            .build();
        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            globalUniformBuffers[i] = createUniformBuffer(globalUniformBufferSize);
        }
    }

    void PostprocessingRenderer::createOrUpdateDescriptorSet(const bool create) {
        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            auto globalBufferInfo = globalUniformBuffers[i]->descriptorInfo(globalUniformBufferSize);
            auto imageInfo        = inputColorAttachmentHdr[i]->imageInfo();
            auto writer           = DescriptorWriter(*setLayout, *descriptorPool)
                                  .writeBuffer(0, &globalBufferInfo)
                                  .writeImage(1, &imageInfo);
            if (create) {
                if (!writer.build(descriptorSet[i])) {
                    die("Cannot allocate descriptor set for BasePostprocessingRenderer");
                }
            } else {
                writer.overwrite(descriptorSet[i]);
            }
        }
    }

    void PostprocessingRenderer::createImagesResources() {
        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            colorAttachmentHdr[i] = make_shared<ColorFrameBufferHDR>(device);
        }
    }

    void PostprocessingRenderer::cleanupImagesResources() {
        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            colorAttachmentHdr[i]->cleanupImagesResources();
        }
    }

    void PostprocessingRenderer::recreateImagesResources() {
        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            colorAttachmentHdr[i]->cleanupImagesResources();
            colorAttachmentHdr[i]->createImagesResources();
        }
    }

    void PostprocessingRenderer::beginRendering(const VkCommandBuffer commandBuffer, const uint32_t currentFrame) {
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

    void PostprocessingRenderer::endRendering(const VkCommandBuffer commandBuffer, const uint32_t currentFrame, const bool isLast) {
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
