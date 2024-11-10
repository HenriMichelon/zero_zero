/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 450

#include "input_datas.glsl"

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec4 tangent;

void main() {
    mat4 model = models.model[pushConstants.modelIndex];
    Material material = materials.material[pushConstants.materialIndex];

    vec3 scaledPosition = position * vec3(1.0f + material.parameters[1].x);
    vec4 globalPosition = model * vec4(scaledPosition, 1.0);
    gl_Position = global.projection * global.view * globalPosition;
}
