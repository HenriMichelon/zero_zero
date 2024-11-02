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

void main() {
    vec3 hdrColor = texture(hdrBuffer, UV).rgb;
    COLOR = vec4(hdrColor, 1.0);
}