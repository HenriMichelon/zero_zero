module;
#include "z0/libraries.h"

export module z0:DirectionalLight;

import :Light;

export namespace z0 {

    /**
     * Directional light from a distance, as from the Sun.
     */
    class DirectionalLight : public Light {
    public:
        /**
         * Create a DirectionalLight
         * @param color RGB color and intensity of the light
         * @param specular intensity of the specular blob in objects affected by the light.
         * @param nodeName Node name
         */
        explicit DirectionalLight(vec4          color    = {1.0f, 1.0f, 1.0f, 1.0f},
                                  float         specular = 1.0f,
                                  const string &nodeName = "DirectionalLight");

        ~DirectionalLight() override = default;

        /**
         * Sets the number of cascades for the shadow map (between 2 and ShadowMapFrameBuffer::CASCADED_SHADOWMAP_MAX_LAYERS).<br>
         * *must* be called before adding the light to the scene since this value is used when instancing the shadow map
         * renderer for this light.
         */
        void setShadowMapCascadesCount(uint32_t cascadesCount);

        /**
         * Returns the number of cascades for the shadow map
         */
        [[nodiscard]] inline uint32_t getShadowMapCascadesCount() const { return shadowMapCascadesCount; }

        void setProperty(const string &property, const string &value) override;

    private:
        uint32_t shadowMapCascadesCount{3};
    };

}
