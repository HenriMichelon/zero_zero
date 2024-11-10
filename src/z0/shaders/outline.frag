/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 460

#include "input_datas.glsl"

layout (location = 0) out vec4 COLOR;

void main() {
    COLOR = materials.material[pushConstants.materialIndex].parameters[0];
}