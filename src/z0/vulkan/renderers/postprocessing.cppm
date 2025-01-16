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
import z0.vulkan.Descriptors;
import z0.vulkan.Device;
import z0.vulkan.Renderer;
import z0.vulkan.Renderpass;

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

        void drawFrame(uint32_t currentFrame, bool isLast) override;

        void createImagesResources() override;

        void cleanupImagesResources() override;

        void recreateImagesResources() override;

        void beginRendering(uint32_t currentFrame);

        void endRendering(uint32_t currentFrame, bool isLast);

    protected:
        vector<shared_ptr<ColorFrameBufferHDR>> colorAttachmentHdr;
        vector<shared_ptr<ColorFrameBufferHDR>> inputColorAttachmentHdr;
    };

}
