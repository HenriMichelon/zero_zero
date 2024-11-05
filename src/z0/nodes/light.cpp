/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

module z0;

import :Light;
import :Tools;

namespace z0 {

    Light::Light(const string &nodeName, const Type type) :
        Node{nodeName, type},
        lightType{type == DIRECTIONAL_LIGHT ? LIGHT_DIRECTIONAL : type == SPOT_LIGHT ? LIGHT_SPOT : LIGHT_OMNI} {
    }

    Light::Light(const vec4 color, const string &nodeName, const Type type):
        Node{nodeName, type},
        colorAndIntensity{color},
        lightType{type == DIRECTIONAL_LIGHT ? LIGHT_DIRECTIONAL : type == SPOT_LIGHT ? LIGHT_SPOT : LIGHT_OMNI} {
    }

    void Light::setCastShadows(const bool castShadows) {
        this->castShadows = castShadows;
    }

    void Light::setProperty(const string &property, const string &value) {
        Node::setProperty(property, value);
        if (property == "color") {
            setColorAndIntensity(to_vec4(value));
        } else if (property == "cast_shadows") {
            setCastShadows(value == "true");
        }
    }

}
