module;
#include "z0/libraries.h"

module z0;

namespace z0 {

    DirectionalLight::DirectionalLight(const string &name):
        Light{name, DIRECTIONAL_LIGHT} {
    }

    DirectionalLight::DirectionalLight(const vec3    direction,
                                       const vec4    color,
                                       const float   specular,
                                       const string &nodeName):
        Light{color, specular, nodeName, DIRECTIONAL_LIGHT},
        direction{normalize(direction)} {
    }

    void DirectionalLight::setProperty(const string &property, const string &value) {
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
