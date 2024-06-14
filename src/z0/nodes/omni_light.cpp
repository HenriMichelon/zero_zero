#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/nodes/light.h"
#include "z0/nodes/omni_light.h"
#endif

namespace z0 {

    OmniLight::OmniLight(float _linear,
                         float _quadratic,
                         float _attenuation,
                         vec4 color, float specular, const std::string nodeName):
            Light{color, specular, nodeName},
            attenuation{_attenuation}, linear{_linear}, quadratic{_quadratic}
    {
    }

}