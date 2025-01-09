/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 450 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec2 UV;

void main() {
    UV = uv;
    vec2 pos = 2 * (position - 0.5); // remap to [-1,1]
    gl_Position = vec4(pos.x, -pos.y, 0, 1);
}
