module;
#include <volk.h>
#include "z0/libraries.h"

export module z0:ShadowMapFrameBuffer;

import :Light;
import :Device;
import :SampledFrameBuffer;

export namespace z0 {

    /**
     * Offscreen frame buffer for rendering shadow, one per light
     */
    class ShadowMapFrameBuffer : public SampledFrameBuffer {
    public:
        explicit ShadowMapFrameBuffer(const Device &dev, const Light *spotLight);

        [[nodiscard]] mat4 getLightSpace(vec3 cameraPosition) const;

        [[nodiscard]] inline const Light *getLight() const { return light; }

        [[nodiscard]] inline vec3 getLightPosition() const { return light->getPositionGlobal(); }

        [[nodiscard]] inline const VkSampler &getSampler() const { return sampler; }

        [[nodiscard]] inline uint32_t getSize() const { return size; }

        void createImagesResources() override;

        void cleanupImagesResources() override;

    private:
        const Light *light;
        bool         lightIsDirectional;
        uint32_t     size;
    };

} // namespace z0
