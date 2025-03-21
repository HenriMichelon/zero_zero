/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 450
#include "utils.glsl"

layout (location = 0) in vec3 NORMAL;
layout (location = 0) out vec4 COLOR;

void main() {
    // Normalize the normal vector
    vec3 normal = normalize(NORMAL);
    // Map the normal vector from [-1, 1] to [0, 1] range for storage in a texture
    COLOR = vec4(normal * 0.5 + 0.5, 0.0f);
}
