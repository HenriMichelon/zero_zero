#pragma once

namespace z0 {

    class ShadowMapFrameBuffer: public BaseFrameBuffer {
    public:
        explicit ShadowMapFrameBuffer(const Device &dev, Light* light);

        const float zNear = .1f;
        const float zFar = 50.0f;
        const uint32_t size{2048};

        mat4 getLightSpace() const;
        vec3 getLightPosition() const { return light->getPosition(); }
        const VkSampler& getSampler() const { return sampler; }

        void createImagesResources();
        void cleanupImagesResources();

    private:
        Light* light;
        VkSampler sampler{VK_NULL_HANDLE};
    };

}