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

        void setProperty(const string &property, const string &value) override;
    };

}
