#pragma once

namespace z0 {

    /**
     * Omnidirectional light, such as a light bulb or a candle
     */
    class OmniLight: public Light {
    public:
        /**
         * Creates an OmniLight with default parameters
         */
        explicit OmniLight(const string name = "OmniLight"): Light{name} {};

        /**
         * Create an OmniLight.
         * @param linear the linear term (see https://learnopengl.com/Lighting/Light-casters)
         * @param quadratic the quadratic term (see https://learnopengl.com/Lighting/Light-casters)
         * @param attenuation the attenuation factor
         * @param color the RGB color and intensity
         * @param specular intensity of the specular blob in objects affected by the light.
         * @param name Node name
         */
        explicit OmniLight(float linear,
                           float quadratic,
                           float attenuation = 1.0f,
                           vec4 color = {1.0f, 1.0f, 1.0f, 1.0f},
                           float specular = 1.0f,
                           const string name = "OmniLight");
        virtual ~OmniLight() = default;

        /**
         * Returns the linear term
         */
        float getLinear() const { return linear; }

        /**
         * Sets the linear term
         */
        void setLinear(float _linear) { linear = _linear; }

        /**
         * Returns the quadratic term
         */
        float getQuadratic() const { return quadratic; }

        /**
         * Sets the quadratic term
         */
        void setQuadratic(float _quadratic) { quadratic = _quadratic;}

        /**
         * Returns the attenuation factor
         */
        float getAttenuation() const { return attenuation; }

        /**
         * Sets the attenuation factor
         */
        void setAttenuation(float _attenuation) { attenuation = _attenuation;}

    private:
        float attenuation{1.0};
        float linear{0.14};
        float quadratic{0.07};
    };

}