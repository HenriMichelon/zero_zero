/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 450
#include "postprocessing_input.glsl"

layout(binding = BINDING_GLOBAL_BUFFER) uniform GobalUniformBufferObject {
    float gamma;
    float exposure;
} global;

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