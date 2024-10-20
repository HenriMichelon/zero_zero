module;
#include <volk.h>
#include "z0/libraries.h"

export module z0:ShadowMapFrameBuffer;

import :Light;
import :Device;
import :SampledFrameBuffer;

export namespace z0{

    /**
     * Offscreen frame buffer for rendering shadow, one per light
     */
    class ShadowMapFrameBuffer : public SampledFrameBuffer {
    public:
        static constexpr uint32_t CASCADED_SHADOWMAP_MAX_LAYERS = 4;

        explicit ShadowMapFrameBuffer(const Device &dev, bool isCascaded);

        [[nodiscard]] inline const VkSampler &getSampler() const { return sampler; }

        [[nodiscard]] inline const VkImageView &getImageView(const uint32_t layer) const { return cascadedImageViews[layer]; };

        [[nodiscard]] inline uint32_t getWidth() const { return width; }
        [[nodiscard]] inline uint32_t getHeight() const { return height; }

        void createImagesResources() override;

        void cleanupImagesResources() override;

    private:
        const bool isCascaded;
        uint32_t width{4096};
        uint32_t height{4096};
        VkImageView cascadedImageViews[CASCADED_SHADOWMAP_MAX_LAYERS];
    };

} // namespace z0
