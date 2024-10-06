module;
#include <volk.h>
#include "z0/libraries.h"

export module z0:ShadowMapFrameBuffer;

import :Light;
import :Device;
import :SampledFrameBuffer;

export namespace z0
{

    /**
     * Offscreen frame buffer for rendering shadow, one per light
     */
    class ShadowMapFrameBuffer : public SampledFrameBuffer
    {
    public:
        explicit ShadowMapFrameBuffer(const Device &dev, bool isCascaded);

        [[nodiscard]] inline const VkSampler &getSampler() const { return sampler; }

        [[nodiscard]] inline uint32_t getSize() const { return size; }

        void createImagesResources() override;

        void cleanupImagesResources() override;

    private:
        uint32_t size;
    };

} // namespace z0
