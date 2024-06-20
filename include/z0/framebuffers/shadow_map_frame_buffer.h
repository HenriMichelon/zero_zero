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

        const Light* getLight() const { return light; }
        mat4 getLightSpace() const;
        vec3 getLightPosition() const { return light->getPosition(); }
        const VkSampler& getSampler() const { return sampler; }

        void createImagesResources();
        void cleanupImagesResources();

    private:
        Light* light;
    };

}