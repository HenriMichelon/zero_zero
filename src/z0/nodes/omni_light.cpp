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
                         const float   specular,
                         const string &nodeName,
                         const Type    type):
        Light{color, specular, nodeName, type}, range{range} {
    }

}
