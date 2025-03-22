/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 450

#include "postprocessing_input.glsl"

void main() {
    vec3 color = texture(normalColor, UV).rgb;
    if (color.r == 1.0f) {
        COLOR = texture(inputImage, UV);
    } else {
        COLOR = vec4(color, 1.0);
    }
}