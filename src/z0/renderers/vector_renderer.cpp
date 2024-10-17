module;
#include "stb_image_write.h"
#include "z0/libraries.h"
#include <volk.h>

module z0;

import :Constants;
import :Tools;
import :Renderer;
import :Renderpass;
import :ColorFrameBufferHDR;
import :Device;
import :Descriptors;
import :Color;
import :Rect;
import :Image;
import :Font;
import :Buffer;
import :Resource;
import :VectorRenderer;

namespace z0 {
    void vr_stb_write_func(void *context, void *data, const int size) {
        auto *buffer = static_cast<vector<unsigned char> *>(context);
        auto *ptr    = static_cast<unsigned char *>(data);
        buffer->insert(buffer->end(), ptr, ptr + size);
    }

    VectorRenderer::VectorRenderer(Device &      device,
                                   const string &shaderDirectory) :
        Renderpass{device, shaderDirectory, WINDOW_CLEAR_COLOR},
        internalColorFrameBuffer{true} {
        init();
    }

    VectorRenderer::VectorRenderer(Device &                               device,
                                   const string &                         shaderDirectory,
                                   const vector<shared_ptr<ColorFrameBufferHDR>> &inputColorAttachmentHdr) :
        Renderpass{device, shaderDirectory, WINDOW_CLEAR_COLOR},
        internalColorFrameBuffer{false} {
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            frameData[i].colorFrameBufferHdr = make_shared<ColorFrameBufferHDR>(device);
        }
        init();
    }

    void VectorRenderer::drawLine(const vec2 start, const vec2 end, const uint32_t currentFrame) {
        auto scaled_start = (start + frameData[currentFrame].translate) / VECTOR_SCALE;
        auto scaled_end   = (end + frameData[currentFrame].translate) / VECTOR_SCALE;
        frameData[currentFrame].vertices.emplace_back(scaled_start);
        frameData[currentFrame].vertices.emplace_back(scaled_end);
        auto color = vec4{vec3{frameData[currentFrame].penColor.color}, std::max(0.0f, frameData[currentFrame].penColor.color.a - frameData[currentFrame].transparency)};
        frameData[currentFrame].commands.emplace_back(PRIMITIVE_LINE, 2, color);
    }

    void VectorRenderer::drawFilledRect(const Rect &rect, const float clip_w, const float clip_h, const uint32_t currentFrame) {
        drawFilledRect(rect.x,
                       rect.y,
                       rect.width,
                       rect.height,
                       clip_w,
                       clip_h,
                       nullptr,
                       currentFrame);
    }

    void VectorRenderer::drawFilledRect(const float              x, const float      y,
                                        const float              w, const float      h,
                                        const float              clip_w, const float clip_h,
                                        const shared_ptr<Image> &texture,
                                        const uint32_t currentFrame) {
        const auto pos  = (vec2{x, y} + frameData[currentFrame].translate) / VECTOR_SCALE;
        vec2       size = vec2{w - 1.0f, h - 1.0f} / VECTOR_SCALE;
        /*
         * v1 ---- v3
         * |  \     |
         * |    \   |
         * v0 ---- v2
         */
        const Vertex v0{vec2{pos.x, pos.y}, vec2{0.0f, 1.0f}};
        const Vertex v1{vec2{pos.x, pos.y + size.y}, vec2{0.0f, 0.0f}};
        const Vertex v2{vec2{pos.x + size.x, pos.y}, vec2{1.0f, 1.0f}};
        const Vertex v3{vec2{pos.x + size.x, pos.y + size.y}, vec2{1.0f, 0.0f}};
        // First triangle
        frameData[currentFrame].vertices.emplace_back(v0);
        frameData[currentFrame].vertices.emplace_back(v1);
        frameData[currentFrame].vertices.emplace_back(v2);
        // Second triangle
        frameData[currentFrame].vertices.emplace_back(v1);
        frameData[currentFrame].vertices.emplace_back(v3);
        frameData[currentFrame].vertices.emplace_back(v2);

        auto color = vec4{vec3{frameData[currentFrame].penColor.color}, std::max(0.0f, frameData[currentFrame].penColor.color.a - frameData[currentFrame].transparency)};
        frameData[currentFrame].commands.emplace_back(PRIMITIVE_RECT, 6, color, texture, clip_w / w, clip_h / h);
        if (texture != nullptr) {
            addImage(texture, currentFrame);
        }
    }

    void VectorRenderer::drawText(const string &text, shared_ptr<Font> &font, const Rect &rect, const float clip_w,
                                  const float   clip_h, const uint32_t currentFrame) {
        //log(text, to_string(rect.width), to_string(clip_w));
        drawText(text, font, rect.x, rect.y, rect.width, rect.height, clip_w, clip_h, currentFrame);
    }

    void VectorRenderer::drawText(const string &text, const shared_ptr<Font> &font, const float x, const float y,
                                  const float   w,
                                  const float   h, const float clip_w,
                                  const float   clip_h, const uint32_t currentFrame) {
        drawFilledRect(x, y, w, h, clip_w, clip_h, font->renderToImage(device, text), currentFrame);
    }

    void VectorRenderer::beginDraw(const uint32_t currentFrame) {
        frameData[currentFrame].vertices.clear();
        frameData[currentFrame].commands.clear();
        frameData[currentFrame].textures.clear();
        frameData[currentFrame].texturesIndices.clear();
    }

    void VectorRenderer::endDraw(const uint32_t currentFrame) {
        // Destroy the previous buffer when we are sure they aren't used by the VkCommandBuffer
        frameData[currentFrame].oldBuffers.clear();
        if (!frameData[currentFrame].vertices.empty()) {
            // Resize the buffers only if needed by recreating them
            if ((frameData[currentFrame].vertexBuffer == VK_NULL_HANDLE) || (frameData[currentFrame].vertexCount != frameData[currentFrame].vertices.size())) {
                // Put the current buffers in the recycle bin since they are currently used by the VkCommandBuffer
                // and can't be destroyed now
                frameData[currentFrame].oldBuffers.push_back(frameData[currentFrame].stagingBuffer);
                frameData[currentFrame].oldBuffers.push_back(frameData[currentFrame].vertexBuffer);
                // Allocate new buffers to change size
                frameData[currentFrame].vertexCount      = frameData[currentFrame].vertices.size();
                frameData[currentFrame].vertexBufferSize = frameData[currentFrame].vertexSize * frameData[currentFrame].vertexCount;
                frameData[currentFrame].stagingBuffer    = make_shared<Buffer>(
                        device,
                        frameData[currentFrame].vertexSize,
                        frameData[currentFrame].vertexCount,
                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT
                        );
                frameData[currentFrame].vertexBuffer = make_shared<Buffer>(
                        device,
                        frameData[currentFrame].vertexSize,
                        frameData[currentFrame].vertexCount,
                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                        );
            }
            // Push new vertices data to GPU memory
            frameData[currentFrame].stagingBuffer->writeToBuffer(frameData[currentFrame].vertices.data());
            frameData[currentFrame].stagingBuffer->copyTo(*(frameData[currentFrame].vertexBuffer), frameData[currentFrame].vertexBufferSize);
        }
        descriptorSetNeedUpdate = true;
        // Initialize or update pipeline layout & descriptors sets if needed
        createOrUpdateResources();
    }

    void VectorRenderer::update(const uint32_t currentFrame) {
        if (frameData[currentFrame].vertices.empty()) { return; }
        uint32_t commandIndex = 0;
        for (const auto &command : frameData[currentFrame].commands) {
            CommandUniformBuffer commandUBO{
                    .color = command.color,
                    .textureIndex = (command.texture == nullptr ? -1 : frameData[currentFrame].texturesIndices[command.texture->getId()]),
                    .clipX = command.clipW,
                    .clipY = 1.0f - command.clipH
            };
            writeUniformBuffer(frameData[currentFrame].commandUniformBuffer, &commandUBO, commandIndex);
            commandIndex += 1;
        }
    }

    void VectorRenderer::beginRendering(const VkCommandBuffer commandBuffer, const uint32_t currentFrame) {
        Device::transitionImageLayout(commandBuffer,
                                      frameData[currentFrame].colorFrameBufferHdr->getImage(),
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                      0,
                                      VK_ACCESS_TRANSFER_WRITE_BIT,
                                      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                      VK_PIPELINE_STAGE_TRANSFER_BIT,
                                      VK_IMAGE_ASPECT_COLOR_BIT);
        const VkRenderingAttachmentInfo colorAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView = frameData[currentFrame].colorFrameBufferHdr->getImageView(),
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode = VK_RESOLVE_MODE_NONE,
                .loadOp = internalColorFrameBuffer ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = clearColor,
        };
        const VkRenderingInfo renderingInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
                .pNext = nullptr,
                .renderArea = {{0, 0}, device.getSwapChainExtent()},
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &colorAttachmentInfo,
                .pDepthAttachment = nullptr,
                .pStencilAttachment = nullptr
        };
        vkCmdBeginRendering(commandBuffer, &renderingInfo);
    }

    void VectorRenderer::endRendering(const VkCommandBuffer commandBuffer, const uint32_t currentFrame, const bool isLast) {
        vkCmdEndRendering(commandBuffer);
        Device::transitionImageLayout(commandBuffer,
                                      frameData[currentFrame].colorFrameBufferHdr->getImage(),
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

    void VectorRenderer::createImagesResources() {
        if (internalColorFrameBuffer) {
            for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                frameData[i].colorFrameBufferHdr = make_shared<ColorFrameBufferHDR>(device);
            }
        }
    }

    void VectorRenderer::cleanupImagesResources() {
        if (internalColorFrameBuffer) {
            for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                frameData[i].colorFrameBufferHdr->cleanupImagesResources();
            }
        }
    }

    void VectorRenderer::recreateImagesResources() {
        cleanupImagesResources();
        if (internalColorFrameBuffer) {
            for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                frameData[i].colorFrameBufferHdr->createImagesResources();
            }
        }
    }

    void VectorRenderer::cleanup() {
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            frameData[i].commands.clear();
            frameData[i].textures.clear();
            frameData[i].vertexBuffer.reset();
            frameData[i].stagingBuffer.reset();
            frameData[i].oldBuffers.clear();
            frameData[i].commandUniformBuffer.reset();
            if (internalColorFrameBuffer) {
                for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    frameData[i].colorFrameBufferHdr->cleanupImagesResources();
                }
            }
        }
        if (blankImage != nullptr) {
            blankImage.reset();
            blankImageData.clear();
        }
        Renderpass::cleanup();
    }

    void VectorRenderer::loadShaders() {
        vertShader = createShader("vector.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
        fragShader = createShader("vector.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void VectorRenderer::createDescriptorSetLayout() {
        descriptorPool = DescriptorPool::Builder(device)
                         .setMaxSets(MAX_FRAMES_IN_FLIGHT)
                         .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT)
                         .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT)
                         .build();

        setLayout = DescriptorSetLayout::Builder(device)
                    .addBinding(0,
                                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                VK_SHADER_STAGE_FRAGMENT_BIT)
                    .addBinding(1,
                                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                VK_SHADER_STAGE_FRAGMENT_BIT,
                                MAX_IMAGES)
                    .build();

        // Create an in-memory default blank image
        if (blankImage == nullptr) {
            const auto data = new unsigned char[1 * 1 * 3];
            data[0]   = 0;
            data[1]   = 0;
            data[2]   = 0;
            stbi_write_jpg_to_func(vr_stb_write_func, &blankImageData, 1, 1, 3, data, 100);
            delete[] data;
            blankImage = make_unique<Image>(device, "Blank", 1, 1, blankImageData.size(), blankImageData.data());
        }
    }

    void VectorRenderer::recordCommands(const VkCommandBuffer commandBuffer, const uint32_t currentFrame) {
        if (frameData[currentFrame].vertices.empty())
            return;

        bindShaders(commandBuffer);
        setViewport(commandBuffer, device.getSwapChainExtent().width, device.getSwapChainExtent().height);

        vkCmdSetRasterizationSamplesEXT(commandBuffer, VK_SAMPLE_COUNT_1_BIT);
        constexpr VkBool32 color_blend_enables[] = {VK_TRUE};
        vkCmdSetColorBlendEnableEXT(commandBuffer, 0, 1, color_blend_enables);
        vkCmdSetAlphaToCoverageEnableEXT(commandBuffer, VK_FALSE);

        vkCmdSetLineWidth(commandBuffer, 1);
        // Some GPU don't support other values, but we need to set them for VK_PRIMITIVE_TOPOLOGY_LINE_LIST
        vkCmdSetVertexInputEXT(commandBuffer,
                               1,
                               &bindingDescription,
                               attributeDescriptions.size(),
                               attributeDescriptions.data());
        vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_FRONT_BIT);
        const VkBuffer     buffers[]       = {frameData[currentFrame].vertexBuffer->getBuffer()};
        constexpr VkDeviceSize vertexOffsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, vertexOffsets);

        uint32_t vertexIndex  = 0;
        uint32_t commandIndex = 0;
        for (const auto &command : frameData[currentFrame].commands) {
            vkCmdSetPrimitiveTopologyEXT(commandBuffer,
                                         command.primitive == PRIMITIVE_LINE
                                         ? VK_PRIMITIVE_TOPOLOGY_LINE_LIST
                                         : VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
            array<uint32_t, 1> offsets = {
                    static_cast<uint32_t>(frameData[currentFrame].commandUniformBuffer->getAlignmentSize() * commandIndex),
            };
            bindDescriptorSets(commandBuffer, currentFrame, offsets.size(), offsets.data());
            vkCmdDraw(commandBuffer, command.count, 1, vertexIndex, 0);
            vertexIndex += command.count;
            commandIndex += 1;
        }
    }

    void VectorRenderer::createOrUpdateDescriptorSet(const bool create) {
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if ((!frameData[i].commands.empty()) && (frameData[i].commandUniformBufferCount != frameData[i].commands.size())) {
                frameData[i].commandUniformBufferCount = frameData[i].commands.size();
                frameData[i].commandUniformBuffer = createUniformBuffer(commandUniformBufferSize, frameData[i].commandUniformBufferCount);
            }
            if (frameData[i].commandUniformBufferCount == 0) {
                frameData[i].commandUniformBufferCount = 1;
                frameData[i].commandUniformBuffer = createUniformBuffer(commandUniformBufferSize, frameData[i].commandUniformBufferCount);
            }
            uint32_t imageIndex = 0;
            for (const auto &image : frameData[i].textures) {
                frameData[i].imagesInfo[imageIndex] = image->_getImageInfo();
                imageIndex += 1;
            }
            // initialize the rest of the image info array with the blank image
            for (uint32_t j = imageIndex; j < frameData[i].imagesInfo.size(); j++) {
                frameData[i].imagesInfo[j] = blankImage->_getImageInfo();
            }
            auto commandBufferInfo = frameData[i].commandUniformBuffer->descriptorInfo(commandUniformBufferSize);
            auto writer            = DescriptorWriter(*setLayout, *descriptorPool)
                          .writeBuffer(0, &commandBufferInfo)
                          .writeImage(1, frameData[i].imagesInfo.data());
            if (create) {
                if (!writer.build(descriptorSet[i]))
                    die("Cannot allocate descriptor set");
            } else {
                writer.overwrite(descriptorSet[i]);
            }
        }
    }

    void VectorRenderer::init() {
        createImagesResources();
        attributeDescriptions.push_back({
                VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
                nullptr,
                0,
                0,
                VK_FORMAT_R32G32_SFLOAT,
                offsetof(Vertex, position)
        });
        attributeDescriptions.push_back({
                VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
                nullptr,
                1,
                0,
                VK_FORMAT_R32G32_SFLOAT,
                offsetof(Vertex, uv)
        });
    }

    void VectorRenderer::addImage(const shared_ptr<Image> &image, const uint32_t currentFrame) {
        if (std::find(frameData[currentFrame].textures.begin(), frameData[currentFrame].textures.end(), image.get()) != frameData[currentFrame].textures.end())
            return;
        if (frameData[currentFrame].textures.size() == MAX_IMAGES)
            die("Maximum images count reached for the vector renderer");
        frameData[currentFrame].texturesIndices[image->getId()] = static_cast<int32_t>(frameData[currentFrame].textures.size());
        frameData[currentFrame].textures.push_back(image.get());
    }

}
