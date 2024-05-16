#pragma once

#include "z0/framebuffers/color_frame_buffer_hdr.h"
#include "z0/renderers/base_renderpass.h"
#include "z0/resources/cubemap.h"
#include "z0/nodes/camera.h"
#include "z0/rect.h"

namespace z0 {

    /*
     * 2D Vector drawing
     * Coordinates system :
     *  - origin : {0, 0} bottom left
     *  - max values : { 1000, 1000 } top right
     */
    class VectorRenderer: public BaseRenderpass, public BaseRenderer {
    public:
        // Default coordinates system scale [ 1000, 1000 ]
        const vec2 SCALE{1000.0f };

        // Used when this renderer is the only renderer
        VectorRenderer(const Device& device,
                       const string& shaderDirectory);
        // USed when this renderer is in a renderer chain
        VectorRenderer(const Device& device,
                       const string& shaderDirectory,
                       shared_ptr<ColorFrameBufferHDR>& inputColorAttachmentHdr);

        // Draw a single fragment point
        void drawPoint(vec2 point);
        // Draw a 1-fragment width line
        void drawLine(vec2 start, vec2 end);
        // Draw a empty rectangle
        //void drawRect(const Rect& rect);
        // Draw a filled rectangle
        void drawFilledRect(const Rect& rect);
        // Draw a filled rectangle
        void drawFilledRect(float x, float y, float w, float h);
        // Change the color of the fragment for the next drawing commands
        void setPenColor(Color color) { penColor = color; }
        // Change the [x,y] translation for the next drawing commands
        void setTranslate(vec2 t) { translate = t; }
        // Change the global transparency for the next drawing commands. Value is substracted from the vertex alpha
        void setTransparency(float a) { transparency = a;}

        // Restart a new drawing session, clearing all the previous datas (vertices and drawing commands)
        void beginDraw();
        // Send the datas of the drawing commands to the GPU
        void endDraw();

        VkImage getImage() const override { return colorFrameBufferHdr->getImage(); }
        void update(uint32_t currentFrame) override;
        void beginRendering(VkCommandBuffer commandBuffer) override;
        void endRendering(VkCommandBuffer commandBuffer, bool isLast)  override;
        void createImagesResources() override;
        void cleanupImagesResources() override;
        void recreateImagesResources() override;
        void cleanup() override;
        void loadShaders() override;
        void createDescriptorSetLayout() override;
        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;
        void createOrUpdateDescriptorSet(bool create) override;

    private:
        // Drawind commands primitives
        enum Primitive {
            PRIMITIVE_NONE,
            PRIMITIVE_POINT,
            PRIMITIVE_LINE,
            PRIMITIVE_RECT,
        };
        // A drawing command
        struct Command {
            Primitive primitive;
            uint32_t  count; // number of vertex for this command
            vec4      color;
        };
        // Shader vertex input datas
        struct Vertex {
            alignas(16) vec2 position;
            alignas(16) vec2 uv;
        };

        // Fragment color for the next drawing commands
        Color           penColor {1.0f,1.0f,1.0f,1.0f};
        // [x,y] translation for the next drawing commands
        vec2            translate {0.0f, 0.0f};
        // Global transparency for the next drawing commands. Value is substracted from the vertex alpha
        float           transparency{0.0f};
        // All drawing commands
        list<Command>   commands;

        struct CommandUniformBuffer {
            vec4    color;
            int     textureIndex;
        };
        // For vkCmdSetVertexInputEXT
        const VkVertexInputBindingDescription2EXT     bindingDescription  {
                .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,
                .binding = 0,
                .stride = sizeof(Vertex),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
                .divisor = 1,
        };
        // For vkCmdSetVertexInputEXT
        vector<VkVertexInputAttributeDescription2EXT> attributeDescriptions {};
        // For vertex buffers allocations
        const uint32_t                                vertexSize{sizeof(Vertex)};
        // All the vertices generated by the drawing commands
        vector<Vertex>                                vertices;
        // We need to update the GPU memory
        bool                                          needRefresh{false};
        // Number of vertices for the currently allocated VkBuffers, used to check if we need to resize the buffers
        uint32_t                                      vertexCount{0};
        // Current VkBuffer memory size
        VkDeviceSize                                  vertexBufferSize{0};
        // Staging vertex buffer used when updating GPU memory
        shared_ptr<Buffer>                            stagingBuffer{VK_NULL_HANDLE};
        // Vertex buffer in GPU memory
        shared_ptr<Buffer>                            vertexBuffer{VK_NULL_HANDLE};
        // Used when we need to postpone the buffers destruction when the are in use by a VkCommandBuffer
        list<shared_ptr<Buffer>>                      oldBuffers;
        // The renderer use is own color attachment, not an attachement from the previous renderer
        bool                                          internalColorFrameBuffer;
        // The color attachemenbt for rendering
        shared_ptr<ColorFrameBufferHDR>               colorFrameBufferHdr{VK_NULL_HANDLE};
        // Maximum number of images supported by this renderer
        static constexpr uint32_t                     MAX_IMAGES = 100;
        // Default blank image
        unique_ptr<Image>                             blankImage{nullptr};
        // Default blank image raw datas
        vector<unsigned char>                         blankImageData;
        // Images infos for descriptor sets, pre-filled with blank images
        array<VkDescriptorImageInfo, MAX_IMAGES>      imagesInfo;
        // Currently allocated command uniform buffer count
        uint32_t                                      commandUniformBufferCount{0};
        // Datas for all the commands of the scene, one buffer for all the commands
        vector<unique_ptr<Buffer>>                    commandUniformBuffers{MAX_FRAMES_IN_FLIGHT};
        // Size of the above uniform buffers
        static constexpr VkDeviceSize                 commandUniformBufferSize{sizeof(CommandUniformBuffer)};

        void init();
    };

}