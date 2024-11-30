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
    // Hack to still compile DebugRenderer when Jolt is compiled without
    #define JPH_DEBUG_RENDERER
    #include <Jolt/Renderer/DebugRenderer.cpp>
    #undef JPH_DEBUG_RENDERER
#endif // !JPH_DEBUG_RENDERER
#include <Jolt/Physics/Collision/RayCast.h>


#include "z0/libraries.h"

module z0.DebugRenderer;

import z0.Application;
import z0.CollisionObject;
import z0.Constants;
import z0.RayCast;
import z0.Tools;

import z0.ColorFrameBufferHDR;
import z0.DepthFrameBuffer;
import z0.Descriptors;
import z0.Device;

namespace z0 {

    DebugRenderer::DebugRenderer(Device &device,
                                 const vector<shared_ptr<ColorFrameBufferHDR>> &inputColorAttachmentHdr,
                                 const vector<shared_ptr<DepthFrameBuffer>>    &depthAttachment,
                                 const bool useDepthTest) :
        Renderpass{device, WINDOW_CLEAR_COLOR},
        useDepthTest{useDepthTest} {
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
        commandPool = device.createCommandPool();
        createOrUpdateResources();
        Initialize();
    }

    void DebugRenderer::cleanup() {
        vkDestroyCommandPool(device.getDevice(), commandPool, nullptr);
        ranges::for_each(frameData, [](FrameData& frame) {
            frame.globalBuffer.reset();
        });
        vertexBuffer.reset();
        stagingBuffer.reset();
        oldBuffers.clear();
        Renderpass::cleanup();
    }

    void DebugRenderer::activateCamera(const shared_ptr<Camera> &camera, uint32_t currentFrame) {
        frameData.at(currentFrame).currentCamera = camera;
        auto cameraPosition = frameData.at(currentFrame).currentCamera->getPositionGlobal();
        SetCameraPos(JPH::Vec3(cameraPosition.x, cameraPosition.y, cameraPosition.z));
    }

    void DebugRenderer::drawRayCasts(const shared_ptr<Node>& scene, const vec4 color, const vec4 collidingColor) {
        for(const auto& raycast : scene->findAllChildren<RayCast>(true)) {
            drawLine(
                raycast->getPositionGlobal(),
                raycast->isColliding() ? raycast->getCollisionPoint() : raycast->toGlobal(raycast->getTarget()),
                raycast->isColliding() ? collidingColor : color);
        }
    }

    void DebugRenderer::drawLine(const vec3 from, const vec3 to, const vec4 color) {
        linesVertices.push_back( {from, color });
        linesVertices.push_back( {to, color });
        vertexBufferDirty = true;
    }

    void DebugRenderer::drawTriangle(const vec3 v1, const vec3 v2, const vec3 v3, const vec4 color) {
        triangleVertices.push_back( {v1, color });
        triangleVertices.push_back( {v2, color });
        triangleVertices.push_back( {v3, color });
        vertexBufferDirty = true;
    }

    void DebugRenderer::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, const JPH::ColorArg inColor) {
        const auto color = vec4{inColor.r, inColor.g, inColor.b, inColor.a} / 255.0;
        linesVertices.push_back( {{ inFrom.GetX(), inFrom.GetY(), inFrom.GetZ() }, color });
        linesVertices.push_back( {{ inTo.GetX(), inTo.GetY(), inTo.GetZ() }, color});
        vertexBufferDirty = true;
    }

    void DebugRenderer::DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, JPH::DebugRenderer::ECastShadow inCastShadow) {
        const auto color = vec4{inColor.r, inColor.g, inColor.b, inColor.a} / 255.0;
        triangleVertices.push_back( {{ inV1.GetX(), inV1.GetY(), inV1.GetZ() }, color });
        triangleVertices.push_back( {{ inV2.GetX(), inV2.GetY(), inV2.GetZ() }, color});
        triangleVertices.push_back( {{ inV3.GetX(), inV3.GetY(), inV3.GetZ() }, color});
        vertexBufferDirty = true;
    }

    void DebugRenderer::update(const uint32_t currentFrame) {
        const auto& frame = frameData.at(currentFrame);
        if (!frame.currentCamera || !Application::get().getDisplayDebug()) { return; }
        // Destroy the previous buffer when we are sure they aren't used by another frame
        oldBuffers.clear();
        if (!linesVertices.empty() || !triangleVertices.empty()) {
            // Resize the buffers only if needed by recreating them
            if ((vertexBuffer == VK_NULL_HANDLE) || (vertexCount != (linesVertices.size() + triangleVertices.size()))) {
                // Put the current buffers in the recycle bin since they are currently used
                // and can't be destroyed now
                oldBuffers.push_back(stagingBuffer);
                oldBuffers.push_back(vertexBuffer);
                // Allocate new buffers to change size
                vertexCount      = linesVertices.size() + triangleVertices.size();
                vertexBufferSize = VERTEX_BUFFER_SIZE * vertexCount;
                stagingBuffer    = make_shared<Buffer>(
                        VERTEX_BUFFER_SIZE,
                        vertexCount,
                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
                vertexBuffer = make_shared<Buffer>(
                        VERTEX_BUFFER_SIZE,
                        vertexCount,
                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
            }
            if (vertexBufferDirty) {
                // Push new vertices data to GPU memory
                if (!linesVertices.empty()) {
                    stagingBuffer->writeToBuffer(linesVertices.data(), linesVertices.size() * sizeof(Vertex));
                }
                if (!triangleVertices.empty()) {
                    stagingBuffer->writeToBuffer(triangleVertices.data(), triangleVertices.size() * sizeof(Vertex), linesVertices.size() * sizeof(Vertex));
                }
                const auto commandBuffer = device.beginOneTimeCommandBuffer();
                stagingBuffer->copyTo(commandBuffer.commandBuffer, *(vertexBuffer), vertexBufferSize);
                device.endOneTimeCommandBuffer(commandBuffer);
                vertexBufferDirty = false;
            }
        }
        const auto globalUbo = GlobalBuffer {
            .projection = frame.currentCamera->getProjection(),
            .view       = frame.currentCamera->getView(),
        };
        writeUniformBuffer(frame.globalBuffer, &globalUbo);
    }

    void DebugRenderer::recordCommands(const VkCommandBuffer commandBuffer, const uint32_t currentFrame) {
        if ((!frameData.at(currentFrame).currentCamera) || !Application::get().getDisplayDebug() || (vertexCount == 0)) { return; }
        bindShaders(commandBuffer);
        setViewport(commandBuffer, device.getSwapChainExtent().width, device.getSwapChainExtent().height);
        vkCmdSetVertexInputEXT(commandBuffer,
                              1,
                              &bindingDescription,
                              attributeDescriptions.size(),
                              attributeDescriptions.data());
        vkCmdSetRasterizationSamplesEXT(commandBuffer, VK_SAMPLE_COUNT_1_BIT);
        vkCmdSetDepthTestEnable(commandBuffer, useDepthTest);
        vkCmdSetDepthWriteEnable(commandBuffer, useDepthTest);
        vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_NONE);
        vkCmdSetLineWidth(commandBuffer, 1);
        const VkBuffer buffers[] = {vertexBuffer->getBuffer()};
        constexpr VkDeviceSize vertexOffsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, vertexOffsets);
        bindDescriptorSets(commandBuffer, currentFrame);
        if (!linesVertices.empty()) {
            vkCmdSetPrimitiveTopologyEXT(commandBuffer, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
            vkCmdDraw(commandBuffer, linesVertices.size(), 1, 0, 0);
        }
        if (!triangleVertices.empty()) {
            vkCmdSetPrimitiveTopologyEXT(commandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
            vkCmdDraw(commandBuffer, triangleVertices.size(), 1, linesVertices.size(), 0);
        }
    }

    void DebugRenderer::beginRendering(const VkCommandBuffer commandBuffer, const uint32_t currentFrame) {
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
            .pDepthAttachment = useDepthTest ? &depthAttachmentInfo : nullptr,
            .pStencilAttachment = nullptr
        };
        vkCmdBeginRendering(commandBuffer, &renderingInfo);
    }

    void DebugRenderer::endRendering(const VkCommandBuffer commandBuffer, const uint32_t currentFrame, const bool isLast) {
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

    void DebugRenderer::startDrawing() {
        NextFrame();
        linesVertices.clear();
        triangleVertices.clear();
    }

    void DebugRenderer::loadShaders() {
        vertShader = createShader("debug.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
        fragShader = createShader("debug.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void DebugRenderer::createDescriptorSetLayout() {
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

    void DebugRenderer::createOrUpdateDescriptorSet(const bool create) {
        for (auto frameIndex = 0; frameIndex < device.getFramesInFlight(); frameIndex++) {
            auto globalBufferInfo = frameData.at(frameIndex).globalBuffer->descriptorInfo(GLOBAL_BUFFER_SIZE);
            auto writer = DescriptorWriter(*setLayout, *descriptorPool)
                            .writeBuffer(0, &globalBufferInfo);
            if (!writer.build(descriptorSet.at(frameIndex), create))
                die("Cannot allocate descriptor set for debug_co renderer");
        }
    }
}
