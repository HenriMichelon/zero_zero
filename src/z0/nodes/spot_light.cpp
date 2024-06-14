#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/nodes/light.h"
#include "z0/nodes/omni_light.h"
#include "z0/nodes/spot_light.h"
#endif

namespace z0 {

    SpotLight::SpotLight(vec3 lightDirection,
            float cutOffDegrees,
            float outerCutOffDegrees,
            float linear,
            float quadratic,
            float attenuation,
            vec4 color,
            float specular,
            const string nodeName):
            OmniLight{linear, quadratic, attenuation, color, specular, nodeName},
            direction{lightDirection},
            fov{radians(outerCutOffDegrees)},
            cutOff{cos(radians(cutOffDegrees))},
            outerCutOff{cos(fov)}
    {
    }

    void SpotLight::setCutOff(float cutOffDegrees) {
        SpotLight::cutOff = cos(radians(cutOffDegrees));
    }

    void SpotLight::setOuterCutOff(float outerCutOffDegrees) {
        fov = radians(outerCutOffDegrees);
        SpotLight::outerCutOff = cos(fov);
    }

    shared_ptr<Node> SpotLight::duplicateInstance() {
        return make_shared<SpotLight>(*this);
    }

}