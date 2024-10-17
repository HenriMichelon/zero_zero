module;
#include "stb_image_write.h"
#include "z0/libraries.h"
#include <volk.h>

export module z0:VectorRenderer;

import :Constants;
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

    /*
     * 2D Vector drawing renderer.<br>
     * Coordinates system :<br>
     *  - origin : {0.0, 0.0} bottom left<br>
     *  - max values : { 1000.0, 1000.0 } top right (use VECTOR_SCALE constant)<br>
     */
    export class VectorRenderer : public Renderpass, public Renderer {
    public:
        // Used when this renderer is the only renderer
        VectorRenderer(Device &      device,
                       const string &shaderDirectory);

        // Used when this renderer is in a renderer chain
        VectorRenderer(Device &                               device,
                       const string &                         shaderDirectory,
                       const vector<shared_ptr<ColorFrameBufferHDR>> &inputColorAttachmentHdr);

        // Draw a 1-fragment width line
        void drawLine(vec2 start, vec2 end, uint32_t currentFrame);

        // Draw a filled rectangle
        void drawFilledRect(const Rect &rect, float clip_w, float clip_h, uint32_t currentFrame);

        // Draw a filled rectangle
        void drawFilledRect(float                    x, float      y,
                            float                    w, float      h,
                            float                    clip_w, float clip_h,
                            const shared_ptr<Image> &texture,
                            uint32_t currentFrame);

        // Draw a rectangle filled with a text
        void drawText(const string &    text,
                      shared_ptr<Font> &font,
                      const Rect &      rect,
                      float             clip_w,
                      float             clip_h,
                      uint32_t          currentFrame);

        void drawText(const string &text, const shared_ptr<Font> &font,
                      float         x, float                      y,
                      float         w, float                      h,
                      float         clip_w, float                 clip_h,
                      uint32_t      currentFrame);

        // Change the color of the fragment for the next drawing commands
        inline void setPenColor(const Color color, const uint32_t currentFrame) { frameData[currentFrame].penColor = color; }

        // Change the [x,y] translation for the next drawing commands
        inline void setTranslate(const vec2 t, const uint32_t currentFrame) { frameData[currentFrame].translate = t; }

        // Change the global transparency for the next drawing commands. Value is subtracted from the vertex alpha
        inline void setTransparency(const float a, const uint32_t currentFrame) { frameData[currentFrame].transparency = a; }

        // Restart a new drawing session, clearing all the previous data (vertices and drawing commands)
        void beginDraw(uint32_t currentFrame);

        // Send the data of the drawing commands to the GPU
        void endDraw(uint32_t currentFrame);

        [[nodiscard]] inline VkImage getImage(const uint32_t currentFrame) const override { return frameData[currentFrame].colorFrameBufferHdr->getImage(); }

        void update(uint32_t currentFrame) override;

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

        // Shader vertex input datas
        struct Vertex {
            alignas(16) vec2 position;
            alignas(16) vec2 uv;
        };

        struct CommandUniformBuffer {
            vec4             color;
            alignas(4) int   textureIndex;
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

        // Maximum number of images supported by this renderer
        static constexpr uint32_t MAX_IMAGES = 100;
        // Size of the commands uniform buffers
        static constexpr VkDeviceSize commandUniformBufferSize{sizeof(CommandUniformBuffer)};

        struct {
            // Fragment color for the next drawing commands
            Color penColor{1.0f, 1.0f, 1.0f, 1.0f};
            // [x,y] translation for the next drawing commands
            vec2 translate{0.0f, 0.0f};
            // Global transparency for the next drawing commands. Value is subtracted from the vertex alpha
            float transparency{0.0f};
            // All drawing commands
            list<Command> commands;

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
            // The color attachment for rendering
            shared_ptr<ColorFrameBufferHDR> colorFrameBufferHdr;
            // Images infos for descriptor sets, pre-filled with blank images
            array<VkDescriptorImageInfo, MAX_IMAGES> imagesInfo;
            // Currently allocated command uniform buffer count
            uint32_t commandUniformBufferCount{0};
            // Datas for all the commands of the scene, one buffer for all the commands
            unique_ptr<Buffer> commandUniformBuffer;
            // All the images used in the scene
            list<Image *> textures;
            // Indices of each images in the descriptor binding
            map<Resource::id_t, int32_t> texturesIndices{};
        } frameData[MAX_FRAMES_IN_FLIGHT];

        // For vkCmdSetVertexInputEXT
        vector<VkVertexInputAttributeDescription2EXT> attributeDescriptions{};
        // The renderer use its own color attachment, not an attachment from the previous renderer
        bool internalColorFrameBuffer;
        // Default blank image
        unique_ptr<Image> blankImage{nullptr};
        // Default blank image raw datas
        vector<unsigned char> blankImageData;

        void init();

        void addImage(const shared_ptr<Image> &image, uint32_t currentFrame);
    };
}
