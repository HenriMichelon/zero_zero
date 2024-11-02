/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 450

layout (location = 0) out vec4 COLOR;
layout (location = 0) in vec2 UV;
layout(set = 0, binding = 0) uniform sampler2D hdrBuffer;

void main()
{
    vec3 hdrColor = texture(hdrBuffer, UV).rgb;
    // Convert to grayscale
    float gray = dot(hdrColor.rgb, vec3(0.299, 0.587, 0.114));
    // Apply sepia tone
    vec3 sepia = vec3(gray) * vec3(1.2, 1.0, 0.8);
    // Clamp values to [0, 1] range
    sepia = clamp(sepia, 0.0, 1.0);
    COLOR = vec4(sepia, 1.0);
}