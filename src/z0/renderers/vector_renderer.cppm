module;
#include "stb_image_write.h"
#include "z0/libraries.h"
#include <volk.h>

export module Z0:VectorRenderer;

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

namespace z0 {
    void vr_stb_write_func(void* context, void* data, int size) {
        auto* buffer = static_cast<vector<unsigned char>*>(context);
        auto* ptr = static_cast<unsigned char*>(data);
        buffer->insert(buffer->end(), ptr, ptr + size);
    }

    /*
     * 2D Vector drawing renderer.<br>
     * Coordinates system :<br>
     *  - origin : {0.0, 0.0} bottom left<br>
     *  - max values : { 1000.0, 1000.0 } top right (use VECTOR_SCALE constant)<br>
     */
    export class VectorRenderer : public Renderpass, public Renderer {
    public:
        // Used when this renderer is the only renderer
        VectorRenderer(Device& device,
                       const string& shaderDirectory) :
            Renderpass{device, shaderDirectory},
            internalColorFrameBuffer{true} {
            init();
        }

        // USed when this renderer is in a renderer chain
        VectorRenderer(Device& device,
                       const string& shaderDirectory,
                       const shared_ptr<ColorFrameBufferHDR>& inputColorAttachmentHdr) :
            Renderpass{device, shaderDirectory},
            internalColorFrameBuffer{false},
            colorFrameBufferHdr{inputColorAttachmentHdr} {
            init();
        }

        // Draw a 1-fragment width line
        void drawLine(const vec2 start, const vec2 end) {
            auto scaled_start = (start + translate) / VECTOR_SCALE;
            auto scaled_end = (end + translate) / VECTOR_SCALE;
            vertices.emplace_back(scaled_start);
            vertices.emplace_back(scaled_end);
            auto color = vec4{vec3{penColor.color}, std::max(0.0f, penColor.color.a - transparency)};
            commands.emplace_back(PRIMITIVE_LINE, 2, color);
        }

        // Draw a filled rectangle
        void drawFilledRect(const Rect& rect, const float clip_w, const float clip_h) {
            drawFilledRect(rect.x,
                           rect.y,
                           rect.width,
                           rect.height,
                           clip_w,
                           clip_h);
        }

        // Draw a filled rectangle
        void drawFilledRect(const float x, const float y,
                            const float w, const float h,
                            const float clip_w, const float clip_h,
                            const shared_ptr<Image>& texture = nullptr) {
            const auto pos = (vec2{x, y} + translate) / VECTOR_SCALE;
            vec2 size = vec2{w - 1.0f, h - 1.0f} / VECTOR_SCALE;
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
            vertices.emplace_back(v0);
            vertices.emplace_back(v1);
            vertices.emplace_back(v2);
            // Second triangle
            vertices.emplace_back(v1);
            vertices.emplace_back(v3);
            vertices.emplace_back(v2);

            auto color = vec4{vec3{penColor.color}, std::max(0.0f, penColor.color.a - transparency)};
            commands.emplace_back(PRIMITIVE_RECT, 6, color, texture, clip_w / w, clip_h / h);
            if (texture != nullptr) {
                addImage(texture);
            }
        }

        // Draw a rectangle filled with a text
        void drawText(const string& text, shared_ptr<Font>& font, const Rect& rect, const float clip_w,
                      const float clip_h) {
            //log(text, to_string(rect.width), to_string(clip_w));
            drawText(text, font, rect.x, rect.y, rect.width, rect.height, clip_w, clip_h);
        }

        void drawText(const string& text, shared_ptr<Font>& font, const float x, const float y, const float w,
                      const float h, const float clip_w,
                      const float clip_h) {
            drawFilledRect(x, y, w, h, clip_w, clip_h, font->renderToImage(device, text));
        }

        // Change the color of the fragment for the next drawing commands
        void setPenColor(const Color color) { penColor = color; }
        // Change the [x,y] translation for the next drawing commands
        void setTranslate(const vec2 t) { translate = t; }
        // Change the global transparency for the next drawing commands. Value is subtracted from the vertex alpha
        void setTransparency(const float a) { transparency = a; }

        // Restart a new drawing session, clearing all the previous datas (vertices and drawing commands)
        void beginDraw() {
            vertices.clear();
            commands.clear();
            textures.clear();
            texturesIndices.clear();
        }

        // Send the datas of the drawing commands to the GPU
        void endDraw() {
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
            }
            descriptorSetNeedUpdate = true;
            // Initialize or update pipeline layout & descriptors sets if needed
            createOrUpdateResources();
        }

        [[nodiscard]] VkImage getImage() const override { return colorFrameBufferHdr->getImage(); }

        void update(const uint32_t currentFrame) override {
            if (vertices.empty()) { return; }
            uint32_t commandIndex = 0;
            for (const auto& command : commands) {
                CommandUniformBuffer commandUBO{
                    .color = command.color,
                    .textureIndex = (command.texture == nullptr ? -1 : texturesIndices[command.texture->getId()]),
                    .clipX = command.clipW,
                    .clipY = 1.0f - command.clipH
                };
                writeUniformBuffer(commandUniformBuffers, currentFrame, &commandUBO, commandIndex);
                commandIndex += 1;
            }
        }

        void beginRendering(VkCommandBuffer commandBuffer) override {
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

        void endRendering(VkCommandBuffer commandBuffer, const bool isLast) override {
            vkCmdEndRendering(commandBuffer);
            Device::transitionImageLayout(commandBuffer,
                                          colorFrameBufferHdr->getImage(),
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

        void createImagesResources() override {
            if (internalColorFrameBuffer) { colorFrameBufferHdr = make_shared<ColorFrameBufferHDR>(device); }
        }

        void cleanupImagesResources() override {
            if (internalColorFrameBuffer) { colorFrameBufferHdr->cleanupImagesResources(); }
        }

        void recreateImagesResources() override {
            cleanupImagesResources();
            if (internalColorFrameBuffer) { colorFrameBufferHdr->createImagesResources(); }
        }

        void cleanup() override {
            commands.clear();
            textures.clear();
            vertexBuffer.reset();
            stagingBuffer.reset();
            oldBuffers.clear();
            commandUniformBuffers.clear();
            if (blankImage != nullptr) {
                blankImage.reset();
                blankImageData.clear();
            }
            if (internalColorFrameBuffer) { colorFrameBufferHdr->cleanupImagesResources(); }
            Renderpass::cleanup();
        }

        void loadShaders() override {
            vertShader = createShader("vector.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
            fragShader = createShader("vector.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
        }

        void createDescriptorSetLayout() override {
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

        void recordCommands(VkCommandBuffer commandBuffer, const uint32_t currentFrame) override {
            if (vertices.empty()) return;

            bindShaders(commandBuffer);
            setViewport(commandBuffer, device.getSwapChainExtent().width, device.getSwapChainExtent().height);

            vkCmdSetRasterizationSamplesEXT(commandBuffer, VK_SAMPLE_COUNT_1_BIT);
            VkBool32 color_blend_enables[] = {VK_TRUE};
            vkCmdSetColorBlendEnableEXT(commandBuffer, 0, 1, color_blend_enables);
            vkCmdSetAlphaToCoverageEnableEXT(commandBuffer, VK_FALSE);

            vkCmdSetLineWidth(commandBuffer, 1);
            // Some GPU don't support other values but we need to set them for VK_PRIMITIVE_TOPOLOGY_LINE_LIST
            vkCmdSetVertexInputEXT(commandBuffer,
                                   1,
                                   &bindingDescription,
                                   attributeDescriptions.size(),
                                   attributeDescriptions.data());
            vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_FRONT_BIT);
            VkBuffer buffers[] = {vertexBuffer->getBuffer()};
            VkDeviceSize vertexOffsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, vertexOffsets);

            uint32_t vertexIndex = 0;
            uint32_t commandIndex = 0;
            for (const auto& command : commands) {
                vkCmdSetPrimitiveTopologyEXT(commandBuffer,
                                             command.primitive == PRIMITIVE_LINE
                                                 ? VK_PRIMITIVE_TOPOLOGY_LINE_LIST
                                                 : VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
                array<uint32_t, 1> offsets = {
                    static_cast<uint32_t>(commandUniformBuffers[currentFrame]->getAlignmentSize() * commandIndex),
                };
                bindDescriptorSets(commandBuffer, currentFrame, offsets.size(), offsets.data());
                vkCmdDraw(commandBuffer, command.count, 1, vertexIndex, 0);
                vertexIndex += command.count;
                commandIndex += 1;
            }
        }

        void createOrUpdateDescriptorSet(const bool create) override {
            if ((!commands.empty()) && (commandUniformBufferCount != commands.size())) {
                commandUniformBufferCount = commands.size();
                createUniformBuffers(commandUniformBuffers, commandUniformBufferSize, commandUniformBufferCount);
            }
            if (commandUniformBufferCount == 0) {
                commandUniformBufferCount = 1;
                createUniformBuffers(commandUniformBuffers, commandUniformBufferSize, commandUniformBufferCount);
            }
            uint32_t imageIndex = 0;
            for (const auto& image : textures) {
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
                }
                else {
                    writer.overwrite(descriptorSet[i]);
                }
            }
        }

    private:
        // Drawing commands primitives
        enum Primitive {
            PRIMITIVE_NONE,
            PRIMITIVE_LINE,
            PRIMITIVE_RECT,
        };

        // A drawing command
        struct Command {
            Primitive primitive;
            uint32_t count; // number of vertex for this command
            vec4 color;
            shared_ptr<Image> texture{nullptr};
            float clipW;
            float clipH;
        };

        // Shader vertex input datas
        struct Vertex {
            alignas(16) vec2 position;
            alignas(16) vec2 uv;
        };

        // Fragment color for the next drawing commands
        Color penColor{1.0f, 1.0f, 1.0f, 1.0f};
        // [x,y] translation for the next drawing commands
        vec2 translate{0.0f, 0.0f};
        // Global transparency for the next drawing commands. Value is substracted from the vertex alpha
        float transparency{0.0f};
        // All drawing commands
        list<Command> commands;

        struct CommandUniformBuffer {
            vec4 color;
            alignas(4) int textureIndex;
            alignas(4) float clipX;
            alignas(4) float clipY;
        };

        // For vkCmdSetVertexInputEXT
        const VkVertexInputBindingDescription2EXT bindingDescription{
            .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            .divisor = 1,
        };
        // For vkCmdSetVertexInputEXT
        vector<VkVertexInputAttributeDescription2EXT> attributeDescriptions{};
        // For vertex buffers allocations
        const uint32_t vertexSize{sizeof(Vertex)};
        // All the vertices generated by the drawing commands
        vector<Vertex> vertices;
        // Number of vertices for the currently allocated VkBuffers, used to check if we need to resize the buffers
        uint32_t vertexCount{0};
        // Current VkBuffer memory size
        VkDeviceSize vertexBufferSize{0};
        // Staging vertex buffer used when updating GPU memory
        shared_ptr<Buffer> stagingBuffer{VK_NULL_HANDLE};
        // Vertex buffer in GPU memory
        shared_ptr<Buffer> vertexBuffer{VK_NULL_HANDLE};
        // Used when we need to postpone the buffers destruction when they are in use by a VkCommandBuffer
        list<shared_ptr<Buffer>> oldBuffers;
        // The renderer use its own color attachment, not an attachment from the previous renderer
        bool internalColorFrameBuffer;
        // The color attachment for rendering
        shared_ptr<ColorFrameBufferHDR> colorFrameBufferHdr{VK_NULL_HANDLE};
        // Maximum number of images supported by this renderer
        static constexpr uint32_t MAX_IMAGES = 100;
        // Default blank image
        unique_ptr<Image> blankImage{nullptr};
        // Default blank image raw datas
        vector<unsigned char> blankImageData;
        // Images infos for descriptor sets, pre-filled with blank images
        array<VkDescriptorImageInfo, MAX_IMAGES> imagesInfo;
        // Currently allocated command uniform buffer count
        uint32_t commandUniformBufferCount{0};
        // Datas for all the commands of the scene, one buffer for all the commands
        vector<unique_ptr<Buffer>> commandUniformBuffers{MAX_FRAMES_IN_FLIGHT};
        // Size of the above uniform buffers
        static constexpr VkDeviceSize commandUniformBufferSize{sizeof(CommandUniformBuffer)};
        // All the images used in the scene
        list<Image*> textures;
        // Indices of each images in the descriptor binding
        map<Resource::id_t, int32_t> texturesIndices{};

        void init() {
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

        void addImage(const shared_ptr<Image>& image) {
            if (std::find(textures.begin(), textures.end(), image.get()) != textures.end()) return;
            if (textures.size() == MAX_IMAGES) die("Maximum images count reached for the vector renderer");
            texturesIndices[image->getId()] = static_cast<int32_t>(textures.size());
            textures.push_back(image.get());
        }
    };
}
