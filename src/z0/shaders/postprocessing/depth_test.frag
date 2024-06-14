#version 450

#include "tools.glsl"

layout (location = 0) out vec4 COLOR;
layout (location = 0) in vec2 UV;
layout(set = 0, binding = 1) uniform sampler2D hdrBuffer;

void main() {
    vec3 hdrColor = texture(hdrBuffer, UV).rgb;
    COLOR = vec4(vec3(LinearizeDepth(hdrColor.r) / far), 1.0);
}