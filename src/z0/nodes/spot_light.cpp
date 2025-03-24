/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

module z0.nodes.SpotLight;

import z0.nodes.Node;

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

    void SpotLight::setCutOff(const float cutOffDegrees) {
        cutOff = cos(radians(cutOffDegrees));
    }

    void SpotLight::setOuterCutOff(const float outerCutOffDegrees) {
        fov         = radians(outerCutOffDegrees);
        outerCutOff = cos(fov);
    }

    shared_ptr<Node> SpotLight::duplicateInstance() const {
        return make_shared<SpotLight>(*this);
    }

    void SpotLight::setProperty(const string &property, const string &value) {
        OmniLight::setProperty(property, value);
        if (property == "cutoff") {
            setCutOff(stof(value));
        } else if (property == "outer_cutoff") {
            setOuterCutOff(stof(value));
        }
    }

}
