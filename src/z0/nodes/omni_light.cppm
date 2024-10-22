module;
#include "z0/libraries.h"

export module z0:OmniLight;

import :Light;

export namespace z0 {

    /**
     * Omnidirectional light, such as a light bulb or a candle
     */
    class OmniLight : public Light {
    public:
        /**
         * Creates an OmniLight with default parameters
         */
        explicit OmniLight(const string &name = "OmniLight", Type type = OMNI_LIGHT);

        /**
         * Create an OmniLight.
         * @param linear the linear term (see https://learnopengl.com/Lighting/Light-casters)
         * @param quadratic the quadratic term (see https://learnopengl.com/Lighting/Light-casters)
         * @param attenuation the attenuation factor
         * @param color the RGB color and intensity
         * @param specular intensity of the specular blob in objects affected by the light.
         * @param nodeName Node name
         */
        explicit OmniLight(float         linear,
                           float         quadratic,
                           float         attenuation = 1.0f,
                           vec4          color       = {1.0f, 1.0f, 1.0f, 1.0f},
                           float         specular    = 1.0f,
                           const string &nodeName    = "OmniLight",
                           Type          type        = OMNI_LIGHT);

        ~OmniLight() override = default;

        /**
         * Returns the linear term
         */
        [[nodiscard]] inline float getLinear() const { return linear; }

        /**
         * Sets the linear term
         */
        inline void setLinear(const float linear) { this->linear = linear; }

        /**
         * Returns the quadratic term
         */
        [[nodiscard]] inline float getQuadratic() const { return quadratic; }

        /**
         * Sets the quadratic term
         */
        inline void setQuadratic(const float quadratic) { this->quadratic = quadratic; }

        /**
         * Returns the attenuation factor
         */
        [[nodiscard]] inline float getAttenuation() const { return attenuation; }

        /**
         * Sets the attenuation factor
         */
        inline void setAttenuation(const float attenuation) { this->attenuation = attenuation; }

        /**
         * Returns the light near clipping distance
         */
        [[nodiscard]] inline float getNearClipDistance() const { return nearDistance; }

        /**
         * Returns the light far clipping distance
         */
        [[nodiscard]] inline float getFarClipDistance() const { return farDistance; }


    protected:
        float attenuation{1.0};
        float linear{0.14};
        float quadratic{0.07};
        // Nearest clipping distance
        float nearDistance{0.1f};
        // Furthest clipping distance
        float farDistance{50.0f}; // // TODO compute them from light distance
    };

}
