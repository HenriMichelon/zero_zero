#pragma once

namespace z0 {

    /**
     * Offscreen frame buffer for rendering shadow, one per light
     */
    class ShadowMapFrameBuffer: public SampledFrameBuffer {
    public:
        explicit ShadowMapFrameBuffer(const Device &dev, Light* light);

        const float zNear = 0.1f;
        const float zFar = 50.0f;
        const uint32_t size{4096};

        [[nodiscard]] mat4 getLightSpace() const;
        [[nodiscard]] inline const Light* getLight() const { return light; }
        [[nodiscard]] inline vec3 getLightPosition() const { return light->getPositionGlobal(); }
        [[nodiscard]] inline const VkSampler& getSampler() const { return sampler; }

        void createImagesResources();
        void cleanupImagesResources();

    private:
        Light* light;
    };

}