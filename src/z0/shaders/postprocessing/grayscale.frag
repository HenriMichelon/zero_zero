/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 450
#include "postprocessing_input.glsl"

void main()  {
    vec3 hdrColor = texture(hdrBuffer, UV).rgb;
    // Convert to grayscale
    float gray = dot(hdrColor.rgb, vec3(0.299, 0.587, 0.114));
    vec3 result = vec3(gray) * vec3(1.0, 1.0, 1.0);
    // Clamp values to [0, 1] range
    result = clamp(result, 0.0, 1.0);
    COLOR = vec4(result, 1.0);
}