/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <mutex>

#include <volk.h>
#include "stb_image_write.h"
#include "z0/libraries.h"

export module z0:VectorRenderer;

import :Constants;
import :Color;
import :Rect;
import :Resource;
import :Image;
import :Font;

import :Renderer;
import :Renderpass;
import :ColorFrameBufferHDR;
import :Device;
import :Descriptors;
import :Buffer;
import :VulkanImage;

namespace z0 {

    /*
     * 2D Vector drawing renderer.<br>
     * Coordinates system :<br>
     *  - origin : {0.0, 0.0} bottom left<br>
     *  - max values : { 1000.0, 1000.0 } top right (use VECTOR_SCALE constant)<br>
     */
    export class VectorRenderer : public Renderpass, public Renderer {
    public:
        // Used when this renderer is the only renderer
        VectorRenderer(Device &device);

        // Used when this renderer is in a renderer chain
        VectorRenderer(Device &                                     device,
                       const vector<shared_ptr<ColorFrameBufferHDR>> &inputColorAttachmentHdr);

        // Draw a 1-fragment width line
        void drawLine(vec2 start, vec2 end);

        // Draw a filled rectangle
        void drawFilledRect(const Rect &rect, float clip_w, float clip_h);

        // Draw a filled rectangle
        void drawFilledRect(float                    x, float      y,
                            float                    w, float      h,
                            float                    clip_w, float clip_h,
                            const shared_ptr<Image> &texture);

        // Draw a rectangle filled with a text
        void drawText(const string &    text, const shared_ptr<Font> &font,
                      const Rect &      rect,
                      float             clip_w,
                      float             clip_h);

        // Draw a rectangle filled with a text
        void drawText(const string &text, const shared_ptr<Font> &font,
                      float         x, float                      y,
                      float         w, float                      h,
                      float         clip_w, float                 clip_h);

        // Change the color of the fragment for the next drawing commands
        inline void setPenColor(const Color color) { penColor = color; }

        // Change the [x,y] translation for the next drawing commands
        inline void setTranslate(const vec2 t) { translate = t; }

        // Change the global transparency for the next drawing commands. Value is subtracted from the vertex alpha
        inline void setTransparency(const float a) { transparency = a; }

        // Restart a new drawing session, clearing all the previous data (vertices and drawing commands)
        void beginDraw();

        // Send the data of the drawing commands to the GPU
        void endDraw();

        [[nodiscard]] inline VkImage getImage(const uint32_t currentFrame) const override { return frameData[currentFrame].colorFrameBufferHdr->getImage(); }

    private:
        // Drawing commands primitives
        enum Primitive {
            PRIMITIVE_NONE,
            PRIMITIVE_LINE,
            PRIMITIVE_RECT,
        };

        // A drawing command
        struct Command {
            Primitive         primitive;
            uint32_t          count; // number of vertex for this command
            vec4              color;
            shared_ptr<Image> texture{nullptr};
            float             clipW;
            float             clipH;
        };

        // Shader vertex input data
        struct Vertex {
            alignas(16) vec2 position;
            alignas(16) vec2 uv;
        };

        struct PushConstants {
            vec4             color;
            alignas(4) int   textureIndex;
            alignas(4) float clipX;
            alignas(4) float clipY;
        };

        // Size of the push constants data
        static constexpr uint32_t PUSH_CONSTANTS_SIZE{sizeof(PushConstants)};
        // Maximum number of images supported by this renderer
        static constexpr uint32_t MAX_IMAGES{100};
        // For vertex buffers allocations
        static constexpr uint32_t VERTEX_BUFFER_SIZE{sizeof(Vertex)};
        static constexpr VkPushConstantRange pushConstantRange {
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .offset = 0,
            .size = PUSH_CONSTANTS_SIZE
        };
        // For vkCmdSetVertexInputEXT
        static constexpr VkVertexInputBindingDescription2EXT bindingDescription{
            .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            .divisor = 1,
        };

        // Fragment color for the next drawing commands
        Color penColor{1.0f, 1.0f, 1.0f, 1.0f};
        // [x,y] translation for the next drawing commands
        vec2 translate{0.0f, 0.0f};
        // Global transparency for the next drawing commands. Value is subtracted from the vertex alpha
        float transparency{0.0f};

        // All drawing commands
        list<Command> commands;
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
        // All the images used in the scene
        list<shared_ptr<VulkanImage>> textures;
        // Indices of each images in the descriptor binding
        map<Resource::id_t, int32_t> texturesIndices{};

        struct FrameData {
            // Read only copy of the commands we have to draw
            list<Command> commands;
            // The color attachment for rendering
            shared_ptr<ColorFrameBufferHDR> colorFrameBufferHdr;
            // Images infos for descriptor sets, pre-filled with blank images
            array<VkDescriptorImageInfo, MAX_IMAGES> imagesInfo;
        };
        vector<FrameData> frameData;

        // For vkCmdSetVertexInputEXT
        vector<VkVertexInputAttributeDescription2EXT> attributeDescriptions{};
        // The renderer use its own color attachment, not an attachment from the previous renderer
        bool internalColorFrameBuffer;
        // Default blank image
        shared_ptr<VulkanImage> blankImage{nullptr};

        void init();

        void addImage(const shared_ptr<Image> &image);

        void beginRendering(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

        void endRendering(VkCommandBuffer commandBuffer, uint32_t currentFrame, bool isLast) override;

        void createImagesResources() override;

        void cleanupImagesResources() override;

        void recreateImagesResources() override;

        void cleanup() override;

        void loadShaders() override;

        void createDescriptorSetLayout() override;

        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

        void createOrUpdateDescriptorSet(bool create) override;
    };
}
