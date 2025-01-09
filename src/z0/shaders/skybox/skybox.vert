/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 450

layout(set = 0, binding = 0) uniform GlobalUniformBufferObject  {
    mat4 projection;
    mat4 view;
    vec4 ambient;
} global;

layout(location = 0) in vec3 position;
layout(location = 0) out vec3 UV;
layout(location = 1) out vec4 AMBIENT;

void main() {
    vec4 pos = global.projection * global.view * vec4(position, 1.0);
    gl_Position = pos.xyww;
    UV = position;
    AMBIENT = global.ambient;
}