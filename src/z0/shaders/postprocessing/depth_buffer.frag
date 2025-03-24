/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 450

#include "postprocessing_input.glsl"

layout(binding = BINDING_GLOBAL_BUFFER) uniform GobalUniformBufferObject {
    float near;
    float far;
} global;

float LinearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0; // back to NDC
    return (2.0 * global.near * global.far) / (global.far + global.near - z * (global.far - global.near));
}

void main() {
    float depth = texture(depthBuffer, UV).r;
    COLOR = vec4(vec3(LinearizeDepth(depth) / global.far), 1.0);
}