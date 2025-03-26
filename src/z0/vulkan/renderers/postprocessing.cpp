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

import z0.resources.Image;

import z0.vulkan.ColorFrameBufferHDR;
import z0.vulkan.Descriptors;
import z0.vulkan.Device;
import z0.vulkan.Renderer;
import z0.vulkan.Renderpass;

namespace z0 {

    PostprocessingRenderer::PostprocessingRenderer(
        Device &                                       device,
        const string&                                  fragShaderName,
        void*                                          globalBufferData,
        const uint32_t                                 globalBufferSize,
        const vector<shared_ptr<ColorFrameBufferHDR>>& inputColorAttachment,
        const vector<shared_ptr<DepthFrameBuffer>>&    depthAttachment,
        const vector<shared_ptr<NormalFrameBuffer>>&   normalColorAttachment,
        const vector<shared_ptr<DiffuseFrameBuffer>>&  diffuseColorAttachment) :
        Renderpass{device, WINDOW_CLEAR_COLOR},
        Renderer{false},
        fragShaderName{fragShaderName},
        globalBufferData{globalBufferData},
        globalBufferSize{globalBufferSize == 0 ? 1 : globalBufferSize},
        inputColorAttachment{inputColorAttachment},
        depthAttachment{depthAttachment},
        normalColorAttachment{normalColorAttachment},
        diffuseColorAttachment{diffuseColorAttachment} {
        outputColorAttachment.resize(device.getFramesInFlight());
        globalBuffer.resize(device.getFramesInFlight());
        createImagesResources();
        createOrUpdateResources(true, &pushConstantRange, 1);
    }

    void PostprocessingRenderer::setInputColorAttachments(const vector<shared_ptr<ColorFrameBufferHDR>> &input) {
        inputColorAttachment = input;
        createOrUpdateDescriptorSet(false);
    }

    void PostprocessingRenderer::cleanup() {
        globalBuffer.clear();
        cleanupImagesResources();
        Renderpass::cleanup();
    }

    void PostprocessingRenderer::loadShaders() {
        vertShader = createShader("quad.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
        fragShader = createShader(fragShaderName + ".frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void PostprocessingRenderer::update(const uint32_t currentFrame) {
        if (globalBufferData != nullptr) {
            writeUniformBuffer(globalBuffer[currentFrame], globalBufferData);
        }
    }

    void PostprocessingRenderer::drawFrame(const uint32_t currentFrame, const bool isLast) {
        beginRendering(currentFrame);
        const auto& commandBuffer = commandBuffers[currentFrame];
        bindShaders(commandBuffer);
        setViewport(commandBuffer, device.getSwapChainExtent().width, device.getSwapChainExtent().height);
        vkCmdSetRasterizationSamplesEXT(commandBuffer, VK_SAMPLE_COUNT_1_BIT);
        vkCmdSetDepthTestEnable(commandBuffer, VK_FALSE);
        vkCmdSetVertexInputEXT(commandBuffer, 0, nullptr, 0, nullptr);
        vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_NONE);
        bindDescriptorSets(commandBuffer, currentFrame);
        const auto pushConstants = PushConstants {
            .texelSize = 1.0f / vec2{device.getSwapChainExtent().width, device.getSwapChainExtent().height},
        };
        vkCmdPushConstants(
               commandBuffer,
               pipelineLayout,
               VK_SHADER_STAGE_FRAGMENT_BIT,
               0,
               PUSH_CONSTANTS_SIZE,
               &pushConstants);
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
        endRendering(currentFrame, isLast);
    }

    void PostprocessingRenderer::createDescriptorSetLayout() {
        descriptorPool = DescriptorPool::Builder(device)
                                 .setMaxSets(device.getFramesInFlight())
                                 .addPoolSize(
                                     VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                     device.getFramesInFlight())
                                 .addPoolSize(
                                     VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                     device.getFramesInFlight() * (BINDING_DIFFUSE_COLOR + 1))
                                 .build();
        setLayout = DescriptorSetLayout::Builder(device)
                            .addBinding(BINDING_GLOBAL_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
                            .addBinding(BINDING_INPUT_COLOR, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
                            .addBinding(BINDING_DEPTH_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
                            .addBinding(BINDING_NORMAL_COLOR, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
                            .addBinding(BINDING_DIFFUSE_COLOR, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
                            .build();
        if (blankImage == nullptr) {
            blankImage = reinterpret_pointer_cast<VulkanImage>(Image::createBlankImage(device));
        }
        for (auto i = 0; i < device.getFramesInFlight(); i++) {
            globalBuffer[i] = createUniformBuffer(globalBufferSize);
        }
    }

    void PostprocessingRenderer::createOrUpdateDescriptorSet(const bool create) {
        for (auto i = 0; i < device.getFramesInFlight(); i++) {
            auto globalBufferInfo = globalBuffer[i]->descriptorInfo(globalBufferSize);
            auto imageInfo = inputColorAttachment[i]->imageInfo();
            auto depthInfo = depthAttachment.empty() ?
                blankImage->getImageInfo() :
                depthAttachment[i]->imageInfo();
            auto normalInfo = normalColorAttachment[i] ?
                normalColorAttachment[i]->imageInfo() :
                blankImage->getImageInfo();
            auto diffuseInfo = diffuseColorAttachment[i] ?
                diffuseColorAttachment[i]->imageInfo() :
                blankImage->getImageInfo();
            auto writer    = DescriptorWriter(*setLayout, *descriptorPool)
                .writeBuffer(BINDING_GLOBAL_BUFFER, &globalBufferInfo)
                .writeImage(BINDING_INPUT_COLOR, &imageInfo)
                .writeImage(BINDING_DEPTH_BUFFER, &depthInfo)
                .writeImage(BINDING_NORMAL_COLOR, &normalInfo)
                .writeImage(BINDING_DIFFUSE_COLOR, &diffuseInfo);
            if (!writer.build(descriptorSet[i], create)) {
                die("Cannot allocate descriptor set for PostprocessingRenderer");
            }
        }
    }

    void PostprocessingRenderer::createImagesResources() {
        for (auto i = 0; i < device.getFramesInFlight(); i++) {
            outputColorAttachment[i] = make_shared<ColorFrameBufferHDR>(device);
        }
    }

    void PostprocessingRenderer::cleanupImagesResources() {
        for (auto i = 0; i < device.getFramesInFlight(); i++) {
            outputColorAttachment[i]->cleanupImagesResources();
        }
    }

    void PostprocessingRenderer::recreateImagesResources() {
        for (auto i = 0; i < device.getFramesInFlight(); i++) {
            outputColorAttachment[i]->cleanupImagesResources();
            outputColorAttachment[i]->createImagesResources();
        }
    }

    void PostprocessingRenderer::beginRendering(const uint32_t currentFrame) {
        const auto& commandBuffer = commandBuffers[currentFrame];
        device.transitionImageLayout(commandBuffer,
                                     outputColorAttachment[currentFrame]->getImage(),
                                     VK_IMAGE_LAYOUT_UNDEFINED,
                                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                     0,
                                     VK_ACCESS_TRANSFER_WRITE_BIT,
                                     VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                     VK_PIPELINE_STAGE_TRANSFER_BIT,
                                     VK_IMAGE_ASPECT_COLOR_BIT);
        const VkRenderingAttachmentInfo colorAttachmentInfo{
            .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
            .imageView = outputColorAttachment[currentFrame]->getImageView(),
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue  = clearColor
        };

        const VkRenderingInfo renderingInfo{
            .sType                = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
            .pNext                = nullptr,
            .renderArea           = {{0, 0}, device.getSwapChainExtent()},
            .layerCount           = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments    = &colorAttachmentInfo,
            .pDepthAttachment     = nullptr,
            .pStencilAttachment   = nullptr
        };
        vkCmdBeginRendering(commandBuffer, &renderingInfo);
    }

    void PostprocessingRenderer::endRendering(const uint32_t currentFrame, const bool isLast) {
        const auto& commandBuffer = commandBuffers[currentFrame];
        vkCmdEndRendering(commandBuffer);
        device.transitionImageLayout(commandBuffer,
                                     outputColorAttachment[currentFrame]->getImage(),
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
