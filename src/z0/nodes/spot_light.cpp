/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

module z0.SpotLight;

import z0.Node;

namespace z0 {

    SpotLight::SpotLight(const string &name):
        OmniLight{name, SPOT_LIGHT} {
    }

    SpotLight::SpotLight(const float   cutOffDegrees,
                         const float   outerCutOffDegrees,
                         const float   range,
                         const vec4    color,
                         const string &nodeName):
        OmniLight{range, color, nodeName, SPOT_LIGHT},
        fov{radians(outerCutOffDegrees)},
        cutOff{cos(radians(cutOffDegrees))},
        outerCutOff{cos(fov)} {
    }

    void SpotLight::setOuterCutOff(const float outerCutOffDegrees) {
        fov         = radians(outerCutOffDegrees);
        outerCutOff = cos(fov);
    }

    shared_ptr<Node> SpotLight::duplicateInstance() {
        return make_shared<SpotLight>(*this);
    }

    void SpotLight::setProperty(const string &property, const string &value) {
        Node::setProperty(property, value);
        if (property == "fov") {
            fov = stof(value);
        } else if (property == "cutoff") {
            setCutOff(stof(value));
        } else if (property == "outer_cutoff") {
            setCastShadows(value == "true");
        }
    }

}
