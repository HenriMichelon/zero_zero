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
        static constexpr auto CASCADED_SHADOWMAP_LAYERS = 4;

        explicit ShadowMapFrameBuffer(const Device &dev, bool isCascaded);

        [[nodiscard]] inline const VkSampler &getSampler() const { return sampler; }

        [[nodiscard]] inline uint32_t getSize() const { return size; }

        [[nodiscard]] inline const VkImageView &getImageView(const uint32_t layer) const { return cascadedImageViews[layer]; };

        void createImagesResources() override;

        void cleanupImagesResources() override;

    private:
        bool isCascaded;
        array<VkImageView, CASCADED_SHADOWMAP_LAYERS> cascadedImageViews{nullptr};
        static constexpr uint32_t size{4096};
    };

} // namespace z0
