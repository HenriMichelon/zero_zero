/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 450

layout(set = 0, binding = 1) uniform samplerCube skybox;
layout (location = 0) in vec3 UV;
layout (location = 1) in vec4 AMBIENT;
layout (location = 0) out vec4 COLOR;

void main() {
    vec4 color = textureLod(skybox, vec3(UV.x, UV.y, -UV.z), 0); // If multiple mip maps level only use the first (for env. cubemap)
    COLOR =  vec4(color.rgb, 1.0);
    //COLOR = vec4(UV.x, UV.y, -UV.z, 1.0);
}