module;
#include "z0/libraries.h"

export module Z0:OmniLight;

import :Tools;
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
        explicit OmniLight(const string &name = "OmniLight", const Type type = OMNI_LIGHT):
            Light{name, type} {
        };

        /**
         * Create an OmniLight.
         * @param linear the linear term (see https://learnopengl.com/Lighting/Light-casters)
         * @param quadratic the quadratic term (see https://learnopengl.com/Lighting/Light-casters)
         * @param attenuation the attenuation factor
         * @param color the RGB color and intensity
         * @param specular intensity of the specular blob in objects affected by the light.
         * @param nodeName Node name
         */
        explicit OmniLight(const float   linear,
                           const float   quadratic,
                           const float   attenuation = 1.0f,
                           const vec4    color       = {1.0f, 1.0f, 1.0f, 1.0f},
                           const float   specular    = 1.0f,
                           const string &nodeName    = "OmniLight",
                           const Type    type        = OMNI_LIGHT):
            Light{color, specular, nodeName, type},
            attenuation{attenuation}, linear{linear}, quadratic{quadratic} {
        }

        ~OmniLight() override = default;

        /**
         * Returns the linear term
         */
        [[nodiscard]] float getLinear() const { return linear; }

        /**
         * Sets the linear term
         */
        void setLinear(const float linear) { this->linear = linear; }

        /**
         * Returns the quadratic term
         */
        [[nodiscard]] float getQuadratic() const { return quadratic; }

        /**
         * Sets the quadratic term
         */
        void setQuadratic(const float quadratic) { this->quadratic = quadratic; }

        /**
         * Returns the attenuation factor
         */
        [[nodiscard]] float getAttenuation() const { return attenuation; }

        /**
         * Sets the attenuation factor
         */
        void setAttenuation(const float attenuation) { this->attenuation = attenuation; }

    private:
        float attenuation{1.0};
        float linear{0.14};
        float quadratic{0.07};
    };

}
