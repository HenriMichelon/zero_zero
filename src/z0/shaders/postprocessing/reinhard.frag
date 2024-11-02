/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 450

layout (location = 0) out vec4 COLOR;
layout (location = 0) in vec2 UV;

layout(set = 0, binding = 0) uniform GobalUniformBufferObject {
    float gamma;
    float exposure;
} global;
layout(set = 0, binding = 1) uniform sampler2D hdrBuffer;
layout(set = 0, binding = 2) uniform sampler2D depthBuffer;

void main() {
    const vec3 hdrColor = texture(hdrBuffer, UV).rgb;
    const float depth = texture(depthBuffer, UV).r;
    vec3 mapped;
    if (depth == 1.0) {
        mapped = hdrColor;
    } else {
        // exposure tone mapping
        mapped = vec3(1.0) - exp(-hdrColor * global.exposure);
    }
    // gamma correction
    COLOR = vec4(pow(mapped, vec3(1.0 / global.gamma)), 1.0);
}