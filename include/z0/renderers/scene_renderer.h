#pragma once

#include "z0/renderers/base_models_renderer.h"
#include "z0/framebuffers/color_frame_buffer.h"
#include "z0/framebuffers/color_frame_buffer_hdr.h"
#include "z0/nodes/camera.h"

#include <map>

namespace z0 {

    class SceneRenderer: public BaseModelsRenderer {
    public:
        struct GobalUniformBuffer {
            mat4 projection{1.0f};
            mat4 view{1.0f};
            vec4 ambient{ 1.0f, 1.0f, 1.0f, .0f }; // RGB + Intensity;
            alignas(16) vec3 cameraPosition;
        };
        struct ModelUniformBuffer {
            mat4 matrix;
        };

        SceneRenderer(const Device& device, std::string shaderDirectory);

        shared_ptr<ColorFrameBufferHDR>& getColorFrameBufferHDR() { return colorFrameBufferHdr; }
        VkImage getImage() const override { return colorFrameBufferHdr->getImage(); }
        VkImageView getImageView() const override { return colorFrameBufferHdr->getImageView(); }

        void cleanup() override;

    private:
        map<Node::id_t, uint32_t> modelsIndices {};
        vector<MeshInstance*> opaquesModels {};

        // Offscreen frame buffers attachements
        ColorFrameBuffer colorFrameBufferMultisampled;
        shared_ptr<ColorFrameBufferHDR> colorFrameBufferHdr;
        shared_ptr<DepthFrameBuffer> resolvedDepthFrameBuffer;

        void update(uint32_t currentFrame) override;
        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;
        void createDescriptorSetLayout() override;
        void loadShaders() override;
        void createImagesResources() override;
        void cleanupImagesResources() override;
        void recreateImagesResources() override;
        void beginRendering(VkCommandBuffer commandBuffer) override;
        void endRendering(VkCommandBuffer commandBuffer, bool isLast) override;

        void drawModels(VkCommandBuffer commandBuffer, uint32_t currentFrame, const vector<MeshInstance*>& modelsToDraw);

    public:
        SceneRenderer(const SceneRenderer&) = delete;
        SceneRenderer &operator=(const SceneRenderer&) = delete;
    };

}