module;
#include "z0/libraries.h"

export module Z0:DirectionalLight;

import :Tools;
import :Light;

export namespace z0 {

    /**
     * Directional light from a distance, as from the Sun.
     */
   class DirectionalLight: public Light {
    public:
        /**
         * Creates a DirectionalLight with defaults parameters
         */
        explicit DirectionalLight(const string name = "DirectionalLight");

        /**
         * Create a DirectionalLight
         * @param direction direction of the light rays
         * @param color RGB color and intensity of the light
         * @param specular intensity of the specular blob in objects affected by the light.
         * @param name Node name
         */
        explicit DirectionalLight(vec3 direction,
                                  vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f },
                                  float specular = 1.0f,
                                  const string name = "DirectionalLight");
        virtual ~DirectionalLight() {};

        /** 
         * Returns the direction of the light
        */
        [[nodiscard]] const vec3& getDirection() const { return direction; }

        /** 
         * Sets the direction of the light
        */
        void setDirection(vec3 lightDirection) { direction = lightDirection; }

        void setProperty(const string&property, const string& value) override;

    private:
        vec3 direction{0.0f, .5f, 1.0f};
    };

    DirectionalLight::DirectionalLight(const string name): Light{name} {};

    DirectionalLight::DirectionalLight(vec3 lightDirection, vec4 color, float specular, const string nodeName):
        Light{color, specular, nodeName},
        direction{normalize(lightDirection)}  {}

    void DirectionalLight::setProperty(const string&property, const string& value) {
     Node::setProperty(property, value);
     if (property == "direction") {
      setDirection(normalize(to_vec3(value)));
     } else if (property == "color") {
      setColorAndIntensity(to_vec4(value));
     } else if (property == "specular") {
      setSpecularIntensity(stof(value));
     } else if (property == "cast_shadow") {
      setCastShadow(value == "true");
     }
    }


}