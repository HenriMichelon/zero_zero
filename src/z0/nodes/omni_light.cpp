module;
#include "z0/libraries.h"

module z0;

import :OmniLight;

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

    void OmniLight::setProperty(const string &property, const string &value) {
        Light::setProperty(property, value);
        if (property == "range") {
            range = stof(value);
        } else if (property == "near") {
            near = stof(value);
        }
    }

}
