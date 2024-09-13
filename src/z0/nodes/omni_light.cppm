module;
#include "z0/libraries.h"

export module Z0:OmniLight;

import :Tools;
import :Light;

export namespace z0 {

    /**
     * Omnidirectional light, such as a light bulb or a candle
     */
    class OmniLight: public Light {
    public:
        /**
         * Creates an OmniLight with default parameters
         */
        explicit OmniLight(const string& name = "OmniLight"): Light{name} {};

        /**
         * Create an OmniLight.
         * @param linear the linear term (see https://learnopengl.com/Lighting/Light-casters)
         * @param quadratic the quadratic term (see https://learnopengl.com/Lighting/Light-casters)
         * @param attenuation the attenuation factor
         * @param color the RGB color and intensity
         * @param specular intensity of the specular blob in objects affected by the light.
         * @param name Node name
         */
        explicit OmniLight(const float _linear,
                           const float _quadratic,
                           const float _attenuation = 1.0f,
                           const vec4 color = {1.0f, 1.0f, 1.0f, 1.0f},
                           const float specular = 1.0f,
                           const string nodeName = "OmniLight"):
            Light{color, specular, nodeName},
            attenuation{_attenuation}, linear{_linear}, quadratic{_quadratic}
        { }
        ~OmniLight() override = default;

        /**
         * Returns the linear term
         */
        [[nodiscard]] float getLinear() const { return linear; }

        /**
         * Sets the linear term
         */
        void setLinear(const float _linear) { linear = _linear; }

        /**
         * Returns the quadratic term
         */
        [[nodiscard]] float getQuadratic() const { return quadratic; }

        /**
         * Sets the quadratic term
         */
        void setQuadratic(const float _quadratic) { quadratic = _quadratic;}

        /**
         * Returns the attenuation factor
         */
        [[nodiscard]] float getAttenuation() const { return attenuation; }

        /**
         * Sets the attenuation factor
         */
        void setAttenuation(const float _attenuation) { attenuation = _attenuation;}

    private:
        float attenuation{1.0};
        float linear{0.14};
        float quadratic{0.07};
    };

}