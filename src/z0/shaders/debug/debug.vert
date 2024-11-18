/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 450 core

layout(set = 0, binding = 0) uniform GlobalUniformBuffer  {
    mat4 projection;
    mat4 view;
} global;

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

layout(location = 0) out vec4 fragColor;

void main()  {
    fragColor = color;
    gl_Position = global.projection * global.view * vec4(position, 1.0);
}
