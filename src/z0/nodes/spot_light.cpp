module;
#include "z0/libraries.h"

module z0;

import :SpotLight;

namespace z0 {

    SpotLight::SpotLight(const string &name):
        OmniLight{name, SPOT_LIGHT} {
    }

    SpotLight::SpotLight(const float   cutOffDegrees,
                         const float   outerCutOffDegrees,
                         const float   range,
                         const vec4    color,
                         const float   specular,
                         const string &nodeName):
        OmniLight{range, color, specular, nodeName, SPOT_LIGHT},
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

}
