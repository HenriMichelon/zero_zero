/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>
#include <Jolt/Jolt.h>
#include <Jolt/Core/Color.h>
#include <Jolt/Renderer/DebugRenderer.h>
#ifndef JPH_DEBUG_RENDERER
    // Hack to still compile DebugRenderer inside the test framework when Jolt is compiled without
    #define JPH_DEBUG_RENDERER
    #include <Jolt/Renderer/DebugRenderer.cpp>
    #undef JPH_DEBUG_RENDERER
#endif // !JPH_DEBUG_RENDERER
#include "z0/libraries.h"

module z0.DebugCollisionObjectsRenderer;

import z0.CollisionObject;
import z0.Constants;
import z0.Tools;

import z0.ColorFrameBufferHDR;
import z0.DepthFrameBuffer;
import z0.Descriptors;
import z0.Device;

namespace z0 {

    DebugCollisionObjectsRenderer::DebugCollisionObjectsRenderer(Device &device,
                                   const vector<shared_ptr<ColorFrameBufferHDR>> &inputColorAttachmentHdr,
                                   const vector<shared_ptr<DepthFrameBuffer>>    &depthAttachment) :
        Renderpass{device, WINDOW_CLEAR_COLOR} {
        frameData.resize(device.getFramesInFlight());
        for (auto i = 0; i < frameData.size(); i++) {
            frameData.at(i).colorFrameBufferHdr = inputColorAttachmentHdr.at(i);
            frameData.at(i).depthFrameBuffer = depthAttachment.at(i);
        }
        attributeDescriptions.push_back({
               VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
               nullptr,
               0,
               0,
               VK_FORMAT_R32G32B32_SFLOAT,
               offsetof(Vertex, position)
        });
        attributeDescriptions.push_back({
               VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
               nullptr,
               1,
               0,
               VK_FORMAT_R32G32B32A32_SFLOAT,
               offsetof(Vertex, color)
        });
        createOrUpdateResources();
        Initialize();
    }

    void DebugCollisionObjectsRenderer::cleanup() {
        ranges::for_each(frameData, [](FrameData& frame) {
            frame.globalBuffer.reset();
        });
        vertexBuffer.reset();
        stagingBuffer.reset();
        oldBuffers.clear();
        Renderpass::cleanup();
    }

    void DebugCollisionObjectsRenderer::activateCamera(const shared_ptr<Camera> &camera, uint32_t currentFrame) {
        frameData.at(currentFrame).currentCamera = camera;
        auto cameraPosition = frameData.at(currentFrame).currentCamera->getPositionGlobal();
        SetCameraPos(JPH::Vec3(cameraPosition.x, cameraPosition.y, cameraPosition.z));
    }

    void DebugCollisionObjectsRenderer::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, const JPH::ColorArg inColor) {
        const auto color = vec4{inColor.r, inColor.g, inColor.b, inColor.a / 2.0};
        vertices.push_back( {{ inFrom.GetX(), inFrom.GetY(), inFrom.GetZ() }, color });
        vertices.push_back( {{ inTo.GetX(), inTo.GetY(), inTo.GetZ() }, color});
    }

    void DebugCollisionObjectsRenderer::update(const uint32_t currentFrame) {
        const auto& frame = frameData.at(currentFrame);
        if (!frame.currentCamera) { return; }
        // Destroy the previous buffer when we are sure they aren't used by the VkCommandBuffer
        oldBuffers.clear();
        if (!vertices.empty()) {
            // Resize the buffers only if needed by recreating them
            if ((vertexBuffer == VK_NULL_HANDLE) || (vertexCount != vertices.size())) {
                // Put the current buffers in the recycle bin since they are currently used by the VkCommandBuffer
                // and can't be destroyed now
                oldBuffers.push_back(stagingBuffer);
                oldBuffers.push_back(vertexBuffer);
                // Allocate new buffers to change size
                vertexCount      = vertices.size();
                vertexBufferSize = VERTEX_BUFFER_SIZE * vertexCount;
                stagingBuffer    = make_shared<Buffer>(
                        device,
                        VERTEX_BUFFER_SIZE,
                        vertexCount,
                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
                vertexBuffer = make_shared<Buffer>(
                        device,
                        VERTEX_BUFFER_SIZE,
                        vertexCount,
                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
            }
            // Push new vertices data to GPU memory
            stagingBuffer->writeToBuffer(vertices.data());
            stagingBuffer->copyTo(*(vertexBuffer), vertexBufferSize);
        }
        const auto globalUbo = GlobalBuffer {
            .projection = frame.currentCamera->getProjection(),
            .view       = frame.currentCamera->getView(),
        };
        writeUniformBuffer(frame.globalBuffer, &globalUbo);
    }

    void DebugCollisionObjectsRenderer::recordCommands(const VkCommandBuffer commandBuffer, const uint32_t currentFrame) {
        if ((!frameData.at(currentFrame).currentCamera) || vertices.empty()) { return; }
        bindShaders(commandBuffer);
        setViewport(commandBuffer, device.getSwapChainExtent().width, device.getSwapChainExtent().height);
        vkCmdSetVertexInputEXT(commandBuffer,
                              1,
                              &bindingDescription,
                              attributeDescriptions.size(),
                              attributeDescriptions.data());
        vkCmdSetRasterizationSamplesEXT(commandBuffer, device.getSamples());
        vkCmdSetRasterizationSamplesEXT(commandBuffer, VK_SAMPLE_COUNT_1_BIT);
        vkCmdSetDepthTestEnable(commandBuffer, VK_TRUE);
        vkCmdSetDepthWriteEnable(commandBuffer, VK_TRUE);
        vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_NONE);
        vkCmdSetLineWidth(commandBuffer, 1);
        vkCmdSetPrimitiveTopologyEXT(commandBuffer, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
        const VkBuffer buffers[] = {vertexBuffer->getBuffer()};
        constexpr VkDeviceSize vertexOffsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, vertexOffsets);
        bindDescriptorSets(commandBuffer, currentFrame);
        vkCmdDraw(commandBuffer, vertices.size(), 1, 0, 0);
    }

    void DebugCollisionObjectsRenderer::beginRendering(const VkCommandBuffer commandBuffer, const uint32_t currentFrame) {
        Device::transitionImageLayout(commandBuffer,
                                      frameData.at(currentFrame).colorFrameBufferHdr->getImage(),
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                      0,
                                      VK_ACCESS_TRANSFER_WRITE_BIT,
                                      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                      VK_PIPELINE_STAGE_TRANSFER_BIT,
                                      VK_IMAGE_ASPECT_COLOR_BIT);
        const VkRenderingAttachmentInfo colorAttachmentInfo{
            .sType          = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
            .imageView      = frameData.at(currentFrame).colorFrameBufferHdr->getImageView(),
            .imageLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .resolveMode    = VK_RESOLVE_MODE_NONE,
            .loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue     = clearColor,
        };
        const VkRenderingAttachmentInfo depthAttachmentInfo{
            .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
            .imageView          = frameData.at(currentFrame).depthFrameBuffer->getImageView(),
            .imageLayout        = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .resolveMode        = VK_RESOLVE_MODE_NONE,
            .loadOp             = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp            = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .clearValue         = depthClearValue,
        };
        const VkRenderingInfo renderingInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
                .pNext = nullptr,
                .renderArea = {{0, 0}, device.getSwapChainExtent()},
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &colorAttachmentInfo,
                .pDepthAttachment = &depthAttachmentInfo,
                .pStencilAttachment = nullptr
        };
        vkCmdBeginRendering(commandBuffer, &renderingInfo);
    }

    void DebugCollisionObjectsRenderer::endRendering(const VkCommandBuffer commandBuffer, const uint32_t currentFrame, const bool isLast) {
        vkCmdEndRendering(commandBuffer);
        Device::transitionImageLayout(commandBuffer,
                                      frameData.at(currentFrame).colorFrameBufferHdr->getImage(),
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      isLast
                                      ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
                                      : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                      0,
                                      isLast ? VK_ACCESS_TRANSFER_READ_BIT : VK_ACCESS_SHADER_READ_BIT,
                                      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                      isLast
                                      ? VK_PIPELINE_STAGE_TRANSFER_BIT
                                      : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                      VK_IMAGE_ASPECT_COLOR_BIT);
    }

    void DebugCollisionObjectsRenderer::startDrawing() {
        NextFrame();
        vertices.clear();
    }

    void DebugCollisionObjectsRenderer::loadShaders() {
        vertShader = createShader("debug_co.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
        fragShader = createShader("debug_co.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void DebugCollisionObjectsRenderer::createDescriptorSetLayout() {
        descriptorPool =
                DescriptorPool::Builder(device)
                        .setMaxSets(device.getFramesInFlight())
                        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 * device.getFramesInFlight())
                        .build();
        setLayout = DescriptorSetLayout::Builder(device)
                        .addBinding(0,
                                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                    VK_SHADER_STAGE_VERTEX_BIT)
                        .build();
        for (auto i = 0; i < device.getFramesInFlight(); i++) {
            frameData.at(i).globalBuffer = createUniformBuffer(GLOBAL_BUFFER_SIZE);
        }
    }

    void DebugCollisionObjectsRenderer::createOrUpdateDescriptorSet(const bool create) {
        for (auto frameIndex = 0; frameIndex < device.getFramesInFlight(); frameIndex++) {
            auto globalBufferInfo = frameData.at(frameIndex).globalBuffer->descriptorInfo(GLOBAL_BUFFER_SIZE);
            auto writer = DescriptorWriter(*setLayout, *descriptorPool)
                            .writeBuffer(0, &globalBufferInfo);
            if (!writer.build(descriptorSet.at(frameIndex), create))
                die("Cannot allocate descriptor set for debug_co renderer");
        }
    }
}
