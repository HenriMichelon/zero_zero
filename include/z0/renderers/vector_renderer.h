#pragma once

#include "z0/framebuffers/color_frame_buffer_hdr.h"
#include "z0/renderers/base_renderpass.h"
#include "z0/resources/cubemap.h"
#include "z0/nodes/camera.h"
#include "z0/rect.h"

namespace z0 {

    class VectorRenderer: public BaseRenderpass, public BaseRenderer {
    public:
        VectorRenderer(const Device& device,
                       const string& shaderDirectory);
        VectorRenderer(const Device& device,
                       const string& shaderDirectory,
                       shared_ptr<ColorFrameBufferHDR>& inputColorAttachmentHdr);

        void drawPoint(vec2 point);
        void drawLine(vec2 start, vec2 end);
        //void drawRect(const Rect& rect);
        void drawFilledRect(const Rect& rect);
        void drawFilledRect(float x, float y, float w, float h);
        void setPenColor(Color color) { penColor = color; }
        void setTranslate(vec2 t) { translate = t; }

        void beginDraw();
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
        void createOrUpdateDescriptorSet(bool create) override;
        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

    private:
        const vec2 SCALE{1000.0f };
        Color penColor {1.0f,1.0f,1.0f,1.0f};
        vec2 translate {0.0f, 0.0f};
        enum Primitive {
            PRIMITIVE_NONE,
            PRIMITIVE_POINT,
            PRIMITIVE_LINE,
            PRIMITIVE_RECT,
        };
        struct Command {
            Primitive primitive;
            uint32_t count;
        };
        Command currentCommand{PRIMITIVE_NONE, 0};
        list<Command> commands;

        void nextCommand(Primitive primitive);

        struct Vertex {
            alignas(16) vec2 position;
            alignas(16) vec4 color;
            alignas(16) vec2 uv;
        };
        const uint32_t vertexSize = sizeof(Vertex);
        const VkVertexInputBindingDescription2EXT bindingDescription  {
                .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,
                .binding = 0,
                .stride = sizeof(Vertex),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
                .divisor = 1,
        };
        vector<VkVertexInputAttributeDescription2EXT> attributeDescriptions {};
        vector<Vertex> vertices;
        bool needRefresh{false};
        uint32_t vertexCount{0};
        VkDeviceSize vertexBufferSize{0};
        shared_ptr<Buffer> stagingBuffer{VK_NULL_HANDLE};
        shared_ptr<Buffer> vertexBuffer{VK_NULL_HANDLE};
        list<shared_ptr<Buffer>> oldBuffers;
        bool internalColorFrameBuffer;
        shared_ptr<ColorFrameBufferHDR> colorFrameBufferHdr{VK_NULL_HANDLE};

        void init();
    };

}