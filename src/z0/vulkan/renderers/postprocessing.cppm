/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/vulkan.h"
#include "z0/libraries.h"

export module z0.vulkan.PostprocessingRenderer;

import z0.Constants;

import z0.vulkan.ColorFrameBufferHDR;
import z0.vulkan.Buffer;
import z0.vulkan.DepthFrameBuffer;
import z0.vulkan.Descriptors;
import z0.vulkan.Device;
import z0.vulkan.DiffuseFrameBuffer;
import z0.vulkan.NormalFrameBuffer;
import z0.vulkan.Renderer;
import z0.vulkan.Renderpass;
import z0.vulkan.Image;

export namespace z0 {

    /*
     * Base class for post-processing renderers
     */
    class PostprocessingRenderer : public Renderpass, public Renderer {
    public:
        PostprocessingRenderer(Device&                                        device,
                               const string&                                  fragShaderName,
                               void*                                          globalBufferData,
                               uint32_t                                       globalBufferSize,
                               const vector<shared_ptr<ColorFrameBufferHDR>>& inputColorAttachment,
                               const vector<shared_ptr<DepthFrameBuffer>>&    depthAttachement,
                               const vector<shared_ptr<NormalFrameBuffer>>&   normalColorAttachement,
                               const vector<shared_ptr<DiffuseFrameBuffer>>&  diffuseColorAttachement);

        void setInputColorAttachments(const vector<shared_ptr<ColorFrameBufferHDR>> &input);

        [[nodiscard]] inline auto& getColorAttachments() { return outputColorAttachment; }

        [[nodiscard]] inline VkImage getImage(const uint32_t currentFrame) const override { return outputColorAttachment[currentFrame]->getImage(); }

        [[nodiscard]] inline VkImageView getImageView(const uint32_t currentFrame) const override { return outputColorAttachment[currentFrame]->getImageView(); }

        void cleanup() override;

        void loadShaders() override;

        void update(uint32_t currentFrame) override;

        void createOrUpdateDescriptorSet(bool create) override;

        void createDescriptorSetLayout() override;

        void drawFrame(uint32_t currentFrame, bool isLast) override;

        void createImagesResources() override;

        void cleanupImagesResources() override;

        void recreateImagesResources() override;

        void beginRendering(uint32_t currentFrame);

        void endRendering(uint32_t currentFrame, bool isLast);

    protected:
        enum Bindings : uint32_t {
            BINDING_GLOBAL_BUFFER= 0,
            BINDING_INPUT_COLOR  = 1,
            BINDING_DEPTH_BUFFER = 2,
            BINDING_NORMAL_COLOR = 3,
            BINDING_DIFFUSE_COLOR= 4,
        };

        const string                                  fragShaderName;
        uint32_t                                      globalBufferSize{1};
        vector<unique_ptr<Buffer>>                    globalBuffer;
        void*                                         globalBufferData{nullptr};
        vector<shared_ptr<ColorFrameBufferHDR>>       outputColorAttachment;
        vector<shared_ptr<ColorFrameBufferHDR>>       inputColorAttachment;
        const vector<shared_ptr<DepthFrameBuffer>>    depthAttachment;
        const vector<shared_ptr<NormalFrameBuffer>>   normalColorAttachment;
        const vector<shared_ptr<DiffuseFrameBuffer>>   diffuseColorAttachment;

        struct PushConstants {
            alignas(8) vec2 texelSize;
        };
        static constexpr uint32_t PUSH_CONSTANTS_SIZE{sizeof(PushConstants)};
        static constexpr VkPushConstantRange pushConstantRange {
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .offset = 0,
            .size = PUSH_CONSTANTS_SIZE
        };

        // Default blank image (for optional frame buffers)
        shared_ptr<VulkanImage> blankImage{nullptr};

    };

}
