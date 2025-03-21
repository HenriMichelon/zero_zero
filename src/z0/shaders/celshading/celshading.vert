/*
 * Copyright (c) 2024-2025 Henri Michelon
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

layout (location = 0) out VertexOut vs_out;

void main() {
    mat4 model = models.model[pushConstants.modelIndex + gl_InstanceIndex];
    vs_out.POSITION = position;
    vs_out.UV = uv;
    vs_out.NORMAL = normalize(mat3(transpose(inverse(model))) * normal);
    vs_out.GLOBAL_POSITION = model * vec4(position, 1.0);
    vs_out.VIEW_DIRECTION = normalize(global.cameraPosition - vs_out.GLOBAL_POSITION.xyz);
    gl_Position = global.projection * global.view * vs_out.GLOBAL_POSITION;
}