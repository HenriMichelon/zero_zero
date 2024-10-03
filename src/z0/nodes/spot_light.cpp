module;
#include "z0/libraries.h"

module z0;

import :SpotLight;

namespace z0 {

    SpotLight::SpotLight(const string &name):
        OmniLight{name, SPOT_LIGHT} {
    }

    SpotLight::SpotLight(const vec3    direction,
                         const float   cutOffDegrees,
                         const float   outerCutOffDegrees,
                         const float   linear,
                         const float   quadratic,
                         const float   attenuation,
                         const vec4    color,
                         const float   specular,
                         const string &nodeName):
        OmniLight{linear, quadratic, attenuation, color, specular, nodeName, SPOT_LIGHT},
        direction{direction},
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
