/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 450 core

layout(location = 0) in vec4 color;

layout(location = 0) out vec4 COLOR;

void main()  {
    COLOR = color;
}
