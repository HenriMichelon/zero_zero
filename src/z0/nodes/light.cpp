module;
#include "z0/libraries.h"

module z0;

import :Light;

namespace z0 {

    Light::Light(const string &nodeName, const Type type) :
        Node{nodeName, type} {
    }

    Light::Light(const vec4 color, const float specular, const string &nodeName, const Type type):
        Node{nodeName, type},
        colorAndIntensity{color},
        specularIntensity{specular} {
    }

    void Light::setCastShadows(const bool castShadows) {
        this->castShadows = castShadows;
    }

    void Light::setProperty(const string &property, const string &value) {
        Node::setProperty(property, value);
        if (property == "color") {
            setColorAndIntensity(to_vec4(value));
        } else if (property == "specular") {
            setSpecularIntensity(stof(value));
        } else if (property == "cast_shadow") {
            setCastShadows(value == "true");
        }
    }

}
