module;
#include "z0/libraries.h"
#include <volk.h>

export module z0:PostprocessingRenderer;

import :Constants;
import :Renderer;
import :Renderpass;
import :Device;
import :ColorFrameBufferHDR;
import :Descriptors;

export namespace z0 {

    /**
     * Base class for post-processing effect renderers
     */
    class PostprocessingRenderer : public Renderpass, public Renderer {
    public:
        PostprocessingRenderer(Device &                             device,
                               const string &                       shaderDirectory,
                               const vector<ColorFrameBufferHDR *> & inputColorAttachment);

        void setInputColorAttachments(const vector<ColorFrameBufferHDR *> &input);

        [[nodiscard]] inline vector<shared_ptr<ColorFrameBufferHDR>> &getColorAttachments() { return colorAttachmentHdr; }

        [[nodiscard]] inline VkImage getImage(const uint32_t currentFrame) const override { return colorAttachmentHdr[currentFrame]->getImage(); }

        [[nodiscard]] inline VkImageView getImageView(const uint32_t currentFrame) const override { return colorAttachmentHdr[currentFrame]->getImageView(); }

        void cleanup() override;

        void loadShaders() override;

        void createOrUpdateDescriptorSet(bool create) override;

        void createDescriptorSetLayout() override;

        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

        void createImagesResources() override;

        void cleanupImagesResources() override;

        void recreateImagesResources() override;

        void beginRendering(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

        void endRendering(VkCommandBuffer commandBuffer, uint32_t currentFrame, bool isLast) override;

    protected:
        vector<shared_ptr<ColorFrameBufferHDR>> colorAttachmentHdr;
        vector<ColorFrameBufferHDR *>           inputColorAttachmentHdr;
    };

}
