#pragma once

namespace z0 {

    /**
     * Base class for post processing effect renderers
     */
    class PostprocessingRenderer: public Renderpass, public Renderer {
    public:
        PostprocessingRenderer(Device& device,
                                   string shaderDirectory,
                                   SampledFrameBuffer* inputColorAttachmentHdr);

        void setInputColorAttachmentHdr(SampledFrameBuffer* inputColorAttachmentHdr);
        [[nodiscard]] shared_ptr<ColorFrameBufferHDR>& getColorAttachment() { return colorAttachmentHdr; }
        [[nodiscard]] VkImage getImage() const override { return colorAttachmentHdr->getImage(); }
        [[nodiscard]] VkImageView getImageView() const override { return colorAttachmentHdr->getImageView(); }

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
        SampledFrameBuffer* inputColorAttachmentHdr;
    };

}