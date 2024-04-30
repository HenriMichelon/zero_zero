#pragma once

#include "z0/tools.h"

namespace z0 {

    struct Color {
        Color() = default;
        explicit Color(vec4 c) { color = c; }
        explicit Color(vec3 c) { color = vec4(c, 1.0); }
        Color(float r, float g, float b) { color = vec4(r, g, b, 1.0f); }
        Color(float r, float g, float b, float a) { color = vec4(r, g, b, a); }

        vec4 color;
    };

}