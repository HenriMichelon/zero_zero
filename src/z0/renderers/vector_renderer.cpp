#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/resources/image.h"
#include "z0/resources/font.h"
#include "z0/renderers/base_renderpass.h"
#include "z0/renderers/vector_renderer.h"
#endif

namespace z0 {

    void vr_stb_write_func(void *context, void *data, int size) {
        auto* buffer = reinterpret_cast<vector<unsigned char>*>(context);
        auto* ptr = static_cast<unsigned char*>(data);
        buffer->insert(buffer->end(), ptr, ptr + size);
    }

    VectorRenderer::VectorRenderer(const Device &dev,
                                   const string& sDir) :
            BaseRenderpass{dev, sDir},
            internalColorFrameBuffer{true} {
        init();
    }

    VectorRenderer::VectorRenderer(const Device &dev,
                                   const string& sDir,
                                   shared_ptr<ColorFrameBufferHDR>& inputColorAttachment) :
            BaseRenderpass{dev, sDir},
            internalColorFrameBuffer{false},
            colorFrameBufferHdr{inputColorAttachment} {
        init();
    }

    void VectorRenderer::cleanup() {
        if (internalColorFrameBuffer) { colorFrameBufferHdr->cleanupImagesResources(); }
    }

    void VectorRenderer::drawPoint(vec2 point) {
        point = (point + translate) / SCALE;
        vertices.emplace_back(point);
        auto color = vec4{vec3{penColor.color}, std::max(0.0f, penColor.color.a - transparency)};
        commands.emplace_back(PRIMITIVE_POINT, 1, color);
    }

    void VectorRenderer::drawLine(vec2 start, vec2 end) {
        start = (start + translate) / SCALE;
        end = (end + translate) / SCALE;
        vertices.emplace_back(start );
        vertices.emplace_back(end);
        auto color = vec4{vec3{penColor.color}, std::max(0.0f, penColor.color.a - transparency)};
        commands.emplace_back(PRIMITIVE_LINE, 2, color);
    }

    void VectorRenderer::drawFilledRect(const Rect& rect) {
        drawFilledRect(static_cast<float>(rect.x),
                       static_cast<float>(rect.y),
                       static_cast<float>(rect.width),
                       static_cast<float>(rect.height));
    }

    void VectorRenderer::drawFilledRect(float x, float y, float w, float h, const shared_ptr<Image>& texture) {
        const auto pos = (vec2{x, y} + translate) / SCALE;
        const auto size = vec2{w, h} / SCALE;
        /*
         * v1 ---- v3
         * |  \     |
         * |    \   |
         * v0 ---- v2
         */
        const Vertex v0{vec2{pos.x, pos.y}, vec2{0.0f, 1.0f}};
        const Vertex v1{vec2{pos.x, pos.y+size.y}, vec2{0.0f, 0.0f}};
        const Vertex v2{vec2{pos.x+size.x, pos.y}, vec2{1.0f, 1.0f}};
        const Vertex v3{vec2{pos.x+size.x, pos.y+size.y}, vec2{1.0f, 0.0f}};
        // First triangle
        vertices.emplace_back(v0);
        vertices.emplace_back(v1);
        vertices.emplace_back(v2);
        // Second triangle
        vertices.emplace_back(v1);
        vertices.emplace_back(v3);
        vertices.emplace_back(v2);

        auto color = vec4{vec3{penColor.color}, std::max(0.0f, penColor.color.a - transparency)};
        commands.emplace_back(PRIMITIVE_RECT, 6, color, texture);
        if (texture != nullptr) { addImage(texture); }
    }

    void VectorRenderer::drawText(const std::string &text, shared_ptr<Font>& font, const Rect& rect) {
        drawText(text, font,
                       static_cast<float>(rect.x),
                       static_cast<float>(rect.y),
                       static_cast<float>(rect.width),
                       static_cast<float>(rect.height));
    }

    void VectorRenderer::drawText(const std::string &text, shared_ptr<Font>& font, float x, float y, float w, float h) {
        drawFilledRect(x, y, w, h, font->renderToImage(device, text));
    }

    void VectorRenderer::beginDraw() {
        vertices.clear();
        commands.clear();
        textures.clear();
        texturesIndices.clear();
    }

    void VectorRenderer::endDraw() {
        // Destroy the previous buffer when we are sure they aren't used by the VkCommandBuffer
        oldBuffers.clear();
        // Resize the buffers only if needed by recreating them
        if ((vertexBuffer == VK_NULL_HANDLE) || (vertexCount != vertices.size())) {
            // Put the current buffers in the recycle bin since they are currently used by the VkCommandBuffer
            // and can't be destroyed now
            oldBuffers.push_back(stagingBuffer);
            oldBuffers.push_back(vertexBuffer);
            // Allocate new buffers to change size
            vertexCount = vertices.size();
            vertexBufferSize = vertexSize * vertexCount;
            stagingBuffer = make_shared<Buffer>(
                    device,
                    vertexSize,
                    vertexCount,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT
            );
            vertexBuffer = make_shared<Buffer>(
                    device,
                    vertexSize,
                    vertexCount,
                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
            );
        }
        // Push new vertices data to GPU memory
        stagingBuffer->writeToBuffer((void*)vertices.data());
        stagingBuffer->copyTo(*vertexBuffer, vertexBufferSize);
        descriptorSetNeedUpdate = true;
        // Initialize or update pipeline layout & descriptos sets if needed
        createOrUpdateResources();
    }

    void VectorRenderer::addImage(const shared_ptr<Image>& image) {
        if (std::find(textures.begin(), textures.end(), image.get()) != textures.end()) return;
        if (textures.size() == MAX_IMAGES) die("Maximum images count reached for the vector renderer");
        texturesIndices[image->getId()] = static_cast<int32_t>(textures.size());
        textures.push_back(image.get());
    }

    void VectorRenderer::loadShaders() {
        vertShader = createShader("vector.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
        fragShader = createShader("vector.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void VectorRenderer::update(uint32_t currentFrame) {
        if (vertices.empty()) { return; }
        uint32_t commandIndex = 0;
        for (const auto& command : commands) {
            CommandUniformBuffer commandUBO {
                .color = command.color,
                .textureIndex = (command.texture == nullptr ? -1 :  texturesIndices[command.texture->getId()])
            };
            writeUniformBuffer(commandUniformBuffers, currentFrame, &commandUBO, commandIndex);
            commandIndex += 1;
        }
    }

    void VectorRenderer::recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) {
        if (vertices.empty()) return;

        bindShaders(commandBuffer);
        setViewport(commandBuffer, device.getSwapChainExtent().width, device.getSwapChainExtent().height);

        vkCmdSetRasterizationSamplesEXT(commandBuffer, VK_SAMPLE_COUNT_1_BIT);
        VkBool32 color_blend_enables[] = {VK_TRUE};
        vkCmdSetColorBlendEnableEXT(commandBuffer, 0, 1, color_blend_enables);
        vkCmdSetAlphaToCoverageEnableEXT(commandBuffer, VK_FALSE);

        vkCmdSetLineWidth(commandBuffer, 1); // Some GPU don't support other values but we need to set them for VK_PRIMITIVE_TOPOLOGY_LINE_LIST
        vkCmdSetVertexInputEXT(commandBuffer,
                               1,
                               &bindingDescription,
                               attributeDescriptions.size(),
                               attributeDescriptions.data());
        vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_FRONT_BIT);
        VkBuffer buffers[] = { vertexBuffer->getBuffer() };
        VkDeviceSize vertexOffsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, vertexOffsets);

        uint32_t vertexIndex = 0;
        uint32_t commandIndex = 0;
        for (const auto& command : commands) {
            vkCmdSetPrimitiveTopologyEXT(commandBuffer,
                                         command.primitive == PRIMITIVE_POINT ? VK_PRIMITIVE_TOPOLOGY_POINT_LIST :
                                         command.primitive == PRIMITIVE_LINE ? VK_PRIMITIVE_TOPOLOGY_LINE_LIST :
                                         VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
            array<uint32_t, 1> offsets = {
                    static_cast<uint32_t>(commandUniformBuffers[currentFrame]->getAlignmentSize() * commandIndex),
            };
            bindDescriptorSets(commandBuffer, currentFrame, offsets.size(), offsets.data());
            vkCmdDraw(commandBuffer, command.count, 1, vertexIndex, 0);
            vertexIndex += command.count;
            commandIndex += 1;
        }
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
            auto data = new unsigned char[1 * 1 * 3];
            data[0] = 0;
            data[1] = 0;
            data[2] = 0;
            stbi_write_jpg_to_func(vr_stb_write_func, &blankImageData, 1, 1, 3, data, 100);
            delete[] data;
            blankImage = make_unique<Image>(device, "Blank", 1, 1, blankImageData.size(), blankImageData.data());
        }
    }

    void VectorRenderer::createOrUpdateDescriptorSet(bool create) {
        if ((!commands.empty()) && (commandUniformBufferCount != commands.size())) {
            commandUniformBufferCount = commands.size();
            createUniformBuffers(commandUniformBuffers, commandUniformBufferSize, commandUniformBufferCount);
        }
        if (commandUniformBufferCount == 0) {
            commandUniformBufferCount = 1;
            createUniformBuffers(commandUniformBuffers, commandUniformBufferSize, commandUniformBufferCount);
        }
        uint32_t imageIndex = 0;
        for(const auto& image : textures) {
            imagesInfo[imageIndex] = image->_getImageInfo();
            imageIndex += 1;
        }
        // initialize the rest of the image info array with the blank image
        for (uint32_t i = imageIndex; i < imagesInfo.size(); i++) {
            imagesInfo[i] = blankImage->_getImageInfo();
        }
        for (uint32_t i = 0; i < descriptorSet.size(); i++) {
            auto commandBufferInfo = commandUniformBuffers[i]->descriptorInfo(commandUniformBufferSize);
            auto writer = DescriptorWriter(*setLayout, *descriptorPool)
                    .writeBuffer(0, &commandBufferInfo)
                    .writeImage(1, imagesInfo.data());
            if (create) {
                if (!writer.build(descriptorSet[i])) die("Cannot allocate descriptor set");
            } else {
                writer.overwrite(descriptorSet[i]);
            }
        }
    }

    void VectorRenderer::recreateImagesResources() {
        cleanupImagesResources();
        if (internalColorFrameBuffer) { colorFrameBufferHdr->createImagesResources(); }
    }

    void VectorRenderer::createImagesResources() {
        if (internalColorFrameBuffer) { colorFrameBufferHdr = make_shared<ColorFrameBufferHDR>(device); }
    }

    void VectorRenderer::cleanupImagesResources() {
        if (internalColorFrameBuffer) { colorFrameBufferHdr->cleanupImagesResources(); }
    }

    void VectorRenderer::beginRendering(VkCommandBuffer commandBuffer) {
        Device::transitionImageLayout(commandBuffer,
                                      colorFrameBufferHdr->getImage(),
                                     VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                     0, VK_ACCESS_TRANSFER_WRITE_BIT,
                                     VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                     VK_IMAGE_ASPECT_COLOR_BIT);
        const VkRenderingAttachmentInfo colorAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView = colorFrameBufferHdr->getImageView(),
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

    void VectorRenderer::endRendering(VkCommandBuffer commandBuffer, bool isLast) {
        vkCmdEndRendering(commandBuffer);
        Device::transitionImageLayout(commandBuffer,
                                      colorFrameBufferHdr->getImage(),
                                     VK_IMAGE_LAYOUT_UNDEFINED,
                                           isLast ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                     0,
                                           isLast ? VK_ACCESS_TRANSFER_READ_BIT : VK_ACCESS_SHADER_READ_BIT,
                                     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                           isLast ? VK_PIPELINE_STAGE_TRANSFER_BIT : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                     VK_IMAGE_ASPECT_COLOR_BIT);
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


}