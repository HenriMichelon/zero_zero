#pragma once

namespace z0 {

    /**
     * Omnidirectional light, such as a light bulb or a candle
     */
    class OmniLight: public Light {
    public:
        explicit OmniLight(const string name = "OmniLight"): Light{name} {};
        explicit OmniLight(float linear,
                           float quadratic,
                           float attenuation = 1.0f,
                           vec4 color = {1.0f, 1.0f, 1.0f, 1.0f},
                           float specular = 1.0f,
                           const string nodeName = "OmniLight");
        virtual ~OmniLight() = default;

        float getLinear() const { return linear; }
        void setLinear(float _linear) { linear = _linear; }
        float getQuadratic() const { return quadratic; }
        void setQuadratic(float _quadratic) { quadratic = _quadratic;}
        float getAttenuation() const { return attenuation; }
        void setAttenuation(float _attenuation) { attenuation = _attenuation;}

    private:
        // http://learnwebgl.brown37.net/09_lights/lights_attenuation.html
        // https://learnopengl.com/Lighting/Light-casters
        float attenuation{1.0};
        float linear{0.14};
        float quadratic{0.07};
    };

}