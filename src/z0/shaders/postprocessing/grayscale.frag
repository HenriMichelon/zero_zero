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
    vec3 result = vec3(gray) * vec3(1.0, 1.0, 1.0);
    // Clamp values to [0, 1] range
    result = clamp(result, 0.0, 1.0);
    COLOR = vec4(result, 1.0);
}