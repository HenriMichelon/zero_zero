/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>
#include "z0/libraries.h"

export module z0.vulkan.ShadowMapFrameBuffer;

import z0.nodes.Light;

import z0.vulkan.Device;
import z0.vulkan.SampledFrameBuffer;

export namespace z0{

    /*
     * Offscreen frame buffer for rendering shadow, one per light
     */
    class ShadowMapFrameBuffer : public SampledFrameBuffer {
    public:
        static constexpr uint32_t CASCADED_SHADOWMAP_MAX_LAYERS = 4;

        explicit ShadowMapFrameBuffer(const Device &dev, bool isCascaded, bool isCubemap);

        [[nodiscard]] inline const VkSampler &getSampler() const { return sampler; }

        [[nodiscard]] inline const VkImageView &getCascadedImageView(const uint32_t layer) const { return cascadedImageViews[layer]; };

        [[nodiscard]] inline uint32_t getWidth() const { return width; }

        [[nodiscard]] inline uint32_t getHeight() const { return height; }

        [[nodiscard]] inline float getRatio() const { return static_cast<float>(width) / static_cast<float>(height); }

        void createImagesResources() override;

        void cleanupImagesResources() override;

    private:
        const bool isCascaded;
        const bool isCubemap;
        uint32_t width{};
        uint32_t height{};
        VkImageView cascadedImageViews[6];
        // index in the uniform buffer array of textures/cubemaps
        int32_t bufferIndex{-1};

    public:
        [[nodiscard]] inline int32_t _getBufferIndex() const { return bufferIndex; }
        inline void _setBufferIndex(const int32_t bufferIndex) { this->bufferIndex = bufferIndex; }
    };

} // namespace z0
