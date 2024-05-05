#include "z0/renderers/vector_renderer.h"

namespace z0 {

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
        if (internalColorFrameBuffer) colorFrameBufferHdr->cleanupImagesResources();
    }

    void VectorRenderer::nextCommand(Primitive primitive) {
        if (currentCommand.primitive != primitive) {
            if (currentCommand.primitive != PRIMITIVE_NONE) {
                auto count = currentCommand.count;
                if (currentCommand.primitive == PRIMITIVE_LINE) count *= 2;
                if (currentCommand.primitive == PRIMITIVE_RECT) count *= 6;
                commands.emplace_back(currentCommand.primitive, count);
            }
            currentCommand.primitive = primitive;
            currentCommand.count = 0;
        }
        currentCommand.count += 1;
    }

    void VectorRenderer::drawPoint(vec2 point) {
        nextCommand(PRIMITIVE_POINT);
        vertices.emplace_back(point, vec4{penColor,transparency});
    }

    void VectorRenderer::drawLine(vec2 start, vec2 end) {
        nextCommand(PRIMITIVE_LINE);
        auto color = vec4{penColor,transparency};
        vertices.emplace_back(start, color);
        vertices.emplace_back(end, color);
    }

    void VectorRenderer::drawRect(vec2 topLeft, vec2 rightBottom) {
        drawLine(topLeft, {rightBottom.x, topLeft.y});
        drawLine({rightBottom.x, topLeft.y}, rightBottom);
        drawLine(rightBottom, {topLeft.x, rightBottom.y});
        drawLine({topLeft.x, rightBottom.y}, topLeft);
    }

    void VectorRenderer::drawFilledRect(vec2 topLeft, vec2 rightBottom) {
        nextCommand(PRIMITIVE_RECT);
        auto color = vec4{penColor,transparency};
        // First triangle
        vertices.emplace_back(topLeft, color);
        vertices.emplace_back(vec2{rightBottom.x, topLeft.y}, color);
        vertices.emplace_back(rightBottom, color);
        // Second triangle
        vertices.emplace_back(topLeft, color);
        vertices.emplace_back(rightBottom, color);
        vertices.emplace_back(vec2{topLeft.x, rightBottom.y}, color);
    }

    void VectorRenderer::beginDraw() {
        vertices.clear();
        commands.clear();
    }

    void VectorRenderer::endDraw() {
        if (vertices.empty()) return;
        nextCommand(PRIMITIVE_NONE);
        vertexCount = vertices.size();
        if ((vertexBuffer == VK_NULL_HANDLE) || (vertexCount != vertices.size())) {
            vertexBufferSize = vertexSize * vertexCount;
            stagingBuffer = make_unique<Buffer>(
                    device,
                    vertexSize,
                    vertexCount,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT
            );
            vertexBuffer = make_unique<Buffer>(
                    device,
                    vertexSize,
                    vertexCount,
                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
            );
        }
        stagingBuffer->writeToBuffer((void*)vertices.data());
        stagingBuffer->copyTo(*vertexBuffer, vertexBufferSize);
    }

    void VectorRenderer::loadShaders() {
        vertShader = createShader("vector.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
        fragShader = createShader("vector.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void VectorRenderer::update(uint32_t currentFrame) {
        if (vertices.empty()) return;
        createOrUpdateResources();
    }

    void VectorRenderer::recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) {
        if (vertices.empty()) return;
        bindShaders(commandBuffer);
        setViewport(commandBuffer, device.getSwapChainExtent().width, device.getSwapChainExtent().height);
        vkCmdSetRasterizationSamplesEXT(commandBuffer, VK_SAMPLE_COUNT_1_BIT);
        vkCmdSetLineWidth(commandBuffer, 1);
        vkCmdSetVertexInputEXT(commandBuffer,
                               1,
                               &bindingDescription,
                               attributeDescriptions.size(),
                               attributeDescriptions.data());
        vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_FRONT_BIT);

        VkBuffer buffers[] = { vertexBuffer->getBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        //bindDescriptorSets(commandBuffer, currentFrame);
        uint32_t vertexIndex = 0;
        for (const auto& command : commands) {
            vkCmdSetPrimitiveTopologyEXT(commandBuffer,
                                         command.primitive == PRIMITIVE_POINT ? VK_PRIMITIVE_TOPOLOGY_POINT_LIST :
                                         command.primitive == PRIMITIVE_LINE ? VK_PRIMITIVE_TOPOLOGY_LINE_LIST :
                                         VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
            vkCmdDraw(commandBuffer, command.count, 1, vertexIndex, 0);
            vertexIndex += command.count;
        }
    }

    void VectorRenderer::createDescriptorSetLayout() {
        descriptorPool = DescriptorPool::Builder(device)
                .setMaxSets(MAX_FRAMES_IN_FLIGHT)
                .build();
        setLayout = DescriptorSetLayout::Builder(device)
                .build();
    }

    void VectorRenderer::createOrUpdateDescriptorSet(bool create) {
    }

    void VectorRenderer::recreateImagesResources() {
        cleanupImagesResources();
        if (internalColorFrameBuffer) colorFrameBufferHdr->createImagesResources();
    }

    void VectorRenderer::createImagesResources() {
        if (internalColorFrameBuffer) colorFrameBufferHdr = make_shared<ColorFrameBufferHDR>(device);
    }

    void VectorRenderer::cleanupImagesResources() {
        if (internalColorFrameBuffer) colorFrameBufferHdr->cleanupImagesResources();
    }

    // https://lesleylai.info/en/vk-khr-dynamic-rendering/
    void VectorRenderer::beginRendering(VkCommandBuffer commandBuffer) {
        Device::transitionImageLayout(commandBuffer,
                                      colorFrameBufferHdr->getImage(),
                                     VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                     0, VK_ACCESS_TRANSFER_WRITE_BIT,
                                     VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                     VK_IMAGE_ASPECT_COLOR_BIT);
        // Color attachement : where the rendering is done (multisampled memory image)
        // Resolved into a non multisampled image
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
                                                2,
                                                0,
                                                VK_FORMAT_R32G32_SFLOAT,
                                                offsetof(Vertex, uv)
                                        });
        attributeDescriptions.push_back({
                                                VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
                                                nullptr,
                                                1,
                                                0,
                                                VK_FORMAT_R32G32B32A32_SFLOAT,
                                                offsetof(Vertex, color)
                                        });
    }


}