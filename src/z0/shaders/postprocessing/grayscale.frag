/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 450
#include "postprocessing_input.glsl"

void main()  {
    const vec3 color = texture(inputImage, UV).rgb;
    // Convert to grayscale
    const float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    vec3 result = vec3(gray) * vec3(1.0, 1.0, 1.0);
    // Clamp values to [0, 1] range
    result = clamp(result, 0.0, 1.0);
    COLOR = vec4(result, 1.0);
}