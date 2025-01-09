/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.vulkan.TonemappingPostprocessingRenderer;

import z0.vulkan.Buffer;
import z0.vulkan.ColorFrameBufferHDR;
import z0.vulkan.DepthFrameBuffer;
import z0.vulkan.Device;
import z0.vulkan.PostprocessingRenderer;

export namespace z0 {

    /*
     * Fragment shader based post processing renderer
     */
    class TonemappingPostprocessingRenderer : public PostprocessingRenderer {
    public:
        TonemappingPostprocessingRenderer(Device& device,
                                     const vector<shared_ptr<ColorFrameBufferHDR>>& inputColorAttachmentHdr,
                                     const vector<shared_ptr<DepthFrameBuffer>>& depthBuffer);

        void loadShaders() override;

        void cleanup() override;

        void update(uint32_t currentFrame) override;

        void createDescriptorSetLayout() override;

        void createOrUpdateDescriptorSet(bool create) override;

    private:
        struct GlobalUniformBuffer {
            alignas(4) float gamma{2.2f};
            alignas(4) float exposure{1.0f};
        };

        vector<unique_ptr<Buffer>>           globalBuffer;
        vector<shared_ptr<DepthFrameBuffer>> resolvedDepthBuffer;
    };
}
