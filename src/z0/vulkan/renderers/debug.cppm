/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>
#include <Jolt/Jolt.h>
#ifdef JPH_DEBUG_RENDERER
#include <Jolt/Renderer/DebugRenderer.h>
#else
    // Hack to still compile DebugRenderer when Jolt is compiled without
    #define JPH_DEBUG_RENDERER
    // Make sure the debug renderer symbols don't get imported or exported
    #define JPH_DEBUG_RENDERER_EXPORT
    #include <Jolt/Renderer/DebugRenderer.h>
    #undef JPH_DEBUG_RENDERER
    #undef JPH_DEBUG_RENDERER_EXPORT
#endif
#include <Jolt/Renderer/DebugRendererSimple.h>
#include "z0/libraries.h"

export module z0.DebugRenderer;

import z0.Camera;
import z0.CollisionObject;
import z0.Node;

import z0.Buffer;
import z0.Device;
import z0.ColorFrameBufferHDR;
import z0.DepthFrameBuffer;
import z0.Renderer;
import z0.Renderpass;

namespace z0 {

    export class DebugRenderer : public Renderpass, public Renderer, public JPH::DebugRendererSimple  {
    public:
        DebugRenderer(Device &device,
                      const vector<shared_ptr<ColorFrameBufferHDR>> &inputColorAttachmentHdr,
                      const vector<shared_ptr<DepthFrameBuffer>>    &depthAttachment,
                      bool useDepthTest);

        void startDrawing();

        void activateCamera(const shared_ptr<Camera> &camera, uint32_t currentFrame);

        void drawRayCasts(const shared_ptr<Node>& scene, vec4 color, vec4 collidingColor);

        void drawLine(vec3 from, vec3 to, vec4 color);

        void drawTriangle(vec3 v1, vec3 v2, vec3 v3, vec4 color);

        void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;

        void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, JPH::DebugRenderer::ECastShadow inCastShadow) override;

        void DrawText3D(JPH::RVec3Arg inPosition, const string_view &inString, JPH::ColorArg inColor, float inHeight) override {};

    private:
        struct GlobalBuffer {
            mat4 projection{1.0f};
            mat4 view{1.0f};
        };
        static constexpr auto GLOBAL_BUFFER_SIZE = sizeof(GlobalBuffer);

        // Shader vertex input data
        struct Vertex {
            alignas(16) vec3 position;
            alignas(16) vec4 color;
        };
        // For vertex buffers allocations
        static constexpr uint32_t VERTEX_BUFFER_SIZE{sizeof(Vertex)};

        // For vkCmdSetVertexInputEXT
        static constexpr VkVertexInputBindingDescription2EXT bindingDescription{
            .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            .divisor = 1,
        };

        struct FrameData {
            shared_ptr<Camera>                 currentCamera;
            shared_ptr<ColorFrameBufferHDR>    colorFrameBufferHdr;
            shared_ptr<DepthFrameBuffer>       depthFrameBuffer;
            unique_ptr<Buffer>                 globalBuffer;
        };
        vector<FrameData> frameData;

        // Use depth testing
        bool useDepthTest{false};
        // Vertex buffer need to be re-uploaded to GPU
        bool vertexBufferDirty = true;
        // All the vertices for lines
        vector<Vertex> linesVertices;
        // All the vertices for triangles
        vector<Vertex> triangleVertices;
        // Number of vertices for the currently allocated VkBuffers, used to check if we need to resize the buffers
        uint32_t vertexCount{0};
        // Current VkBuffer memory size
        VkDeviceSize vertexBufferSize{0};
        // Staging vertex buffer used when updating GPU memory
        shared_ptr<Buffer> stagingBuffer{VK_NULL_HANDLE};
        // Vertex buffer in GPU memory
        shared_ptr<Buffer> vertexBuffer{VK_NULL_HANDLE};
        // Used when we need to postpone the buffers destruction when they are in use by another frame in flight
        list<shared_ptr<Buffer>> oldBuffers;
        // For vkCmdSetVertexInputEXT
        vector<VkVertexInputAttributeDescription2EXT> attributeDescriptions{};
        // To upload vertices to GPU
        VkCommandPool commandPool;

        void update( uint32_t currentFrame) override;

        void cleanup() override;

        void beginRendering(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

        void endRendering(VkCommandBuffer commandBuffer, uint32_t currentFrame, bool isLast) override;

        void loadShaders() override;

        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

        void createDescriptorSetLayout() override;

        void createOrUpdateDescriptorSet(bool create) override;

    };
}
