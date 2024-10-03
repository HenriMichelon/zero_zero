module;
#include "z0/libraries.h"
#include <volk.h>

export module z0:PostprocessingRenderer;

import :Renderer;
import :Renderpass;
import :Device;
import :SampledFrameBuffer;
import :ColorFrameBufferHDR;
import :Descriptors;

export namespace z0 {
    /**
     * Base class for post-processing effect renderers
     */
    class PostprocessingRenderer : public Renderpass, public Renderer {
    public:
        PostprocessingRenderer(Device &            device,
                               const string &      shaderDirectory,
                               SampledFrameBuffer *inputColorAttachmentHdr);

        void setInputColorAttachmentHdr(SampledFrameBuffer *input);

        [[nodiscard]] inline shared_ptr<ColorFrameBufferHDR> &getColorAttachment() { return colorAttachmentHdr; }

        [[nodiscard]] inline VkImage getImage() const override { return colorAttachmentHdr->getImage(); }

        [[nodiscard]] inline VkImageView getImageView() const override { return colorAttachmentHdr->getImageView(); }

        void cleanup() override;

        void loadShaders() override;

        void createOrUpdateDescriptorSet(bool create) override;

        void createDescriptorSetLayout() override;

        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

        void createImagesResources() override;

        void cleanupImagesResources() override;

        void recreateImagesResources() override;

        void beginRendering(VkCommandBuffer commandBuffer) override;

        void endRendering(VkCommandBuffer commandBuffer, bool isLast) override;

    protected:
        shared_ptr<ColorFrameBufferHDR> colorAttachmentHdr;
        SampledFrameBuffer *            inputColorAttachmentHdr;
    };

}
