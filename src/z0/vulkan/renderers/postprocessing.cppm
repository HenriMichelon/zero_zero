/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"
#include <volk.h>

export module z0.PostprocessingRenderer;

import z0.Constants;
import z0.Renderer;
import z0.Renderpass;
import z0.Device;
import z0.ColorFrameBufferHDR;
import z0.Descriptors;

export namespace z0 {

    /*
     * Base class for post-processing effect renderers
     */
    class PostprocessingRenderer : public Renderpass, public Renderer {
    public:
        PostprocessingRenderer(Device &                                        device,
                               const vector<shared_ptr<ColorFrameBufferHDR>> & inputColorAttachment);

        void setInputColorAttachments(const vector<shared_ptr<ColorFrameBufferHDR>> &input);

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
        vector<shared_ptr<ColorFrameBufferHDR>> inputColorAttachmentHdr;
    };

}
