module;
#include "z0/libraries.h"
#include <volk.h>

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
        explicit ShadowMapFrameBuffer(const Device &dev, const Light *spotLight, vec3 position);

        const float    zNear = 0.1f;
        const float    zFar  = 50.0f;
        const uint32_t size{4096};

        [[nodiscard]] mat4 getLightSpace() const;

        [[nodiscard]] inline const Light *getLight() const { return light; }

        [[nodiscard]] inline vec3 getLightPosition() const { return light->getPositionGlobal(); }

        [[nodiscard]] inline const VkSampler &getSampler() const { return sampler; }

        inline void setGlobalPosition(const vec3 position) { globalPosition = position; }

        void createImagesResources() override;

        void cleanupImagesResources() override;

    private:
        const Light *light;
        vec3   globalPosition;
    };

}
