/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 450

#include "postprocessing_input.glsl"

void main() {
    vec3 hdrColor = texture(hdrBuffer, UV).rgb;
    COLOR = vec4(hdrColor, 1.0);
}