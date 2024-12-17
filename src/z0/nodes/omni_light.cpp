/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

module z0.nodes.OmniLight;

import z0.nodes.Node;

namespace z0 {

    OmniLight::OmniLight(const string &name, const Type type):
        Light{name, type} {
    }

    OmniLight::OmniLight(const float   range,
                         const vec4    color,
                         const string &nodeName,
                         const Type    type):
        Light{color, nodeName, type}, range{range} {
    }

    void OmniLight::setRange(const float range) {
        this->range = range;
    }

    void OmniLight::setProperty(const string &property, const string &value) {
        Light::setProperty(property, value);
        if (property == "range") {
            range = stof(value);
        } else if (property == "near") {
            near = stof(value);
        }
    }

    shared_ptr<Node> OmniLight::duplicateInstance() {
        return make_shared<OmniLight>(*this);
    }

}
