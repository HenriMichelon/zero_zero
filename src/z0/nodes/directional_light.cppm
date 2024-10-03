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
         * Creates a DirectionalLight with defaults parameters
         */
        explicit DirectionalLight(const string &name = "DirectionalLight");

        /**
         * Create a DirectionalLight
         * @param direction direction of the light rays
         * @param color RGB color and intensity of the light
         * @param specular intensity of the specular blob in objects affected by the light.
         * @param nodeName Node name
         */
        explicit DirectionalLight(vec3          direction,
                                  vec4          color    = {1.0f, 1.0f, 1.0f, 1.0f},
                                  float         specular = 1.0f,
                                  const string &nodeName = "DirectionalLight");

        ~DirectionalLight() override = default;

        /** 
         * Returns the direction of the light
        */
        [[nodiscard]] inline const vec3 &getDirection() const { return direction; }

        /** 
         * Sets the direction of the light
        */
        inline void setDirection(const vec3 lightDirection) { direction = lightDirection; }

        void setProperty(const string &property, const string &value) override;

    private:
        vec3 direction{0.0f, .5f, 1.0f};
    };

}
