module;
#include "z0/libraries.h"
#include <volk.h>

export module z0:Equirect2CubemapRenderer;

import :Constants;
import :Renderer;
import :Renderpass;
import :Device;
import :ColorFrameBufferHDR;
import :Descriptors;
import :Image;

export namespace z0 {

    /**
     * Converts equirectangular projection texture into a cubemap.
     */
    class Equirect2CubemapRenderer : public Renderpass, public Renderer {
    public:
        Equirect2CubemapRenderer(Device &                                        device,
                                 const shared_ptr<Image>&                        hdrFile,
                                 const string &                                  shaderDirectory);

        void cleanup() override;

        void loadShaders() override;

        void createOrUpdateDescriptorSet(bool create) override;

        void createDescriptorSetLayout() override;

        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

        void beginRendering(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

        void endRendering(VkCommandBuffer commandBuffer, uint32_t currentFrame, bool isLast) override;

    protected:
        const shared_ptr<Image>& hdrFile;
    };

}
