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
    COLOR = vec4(normalize(NORMAL), 0.0f);
}
