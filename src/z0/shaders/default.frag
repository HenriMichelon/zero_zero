#version 450

#include "input_datas.glsl"

layout (location = 0) in VertexOut fs_in;
layout (location = 0) out vec4 COLOR;

vec4 color;

void main() {
    //COLOR = vec4(fs_in.UV.x, fs_in.UV.y, 0.0, 1.0);
    if (material.diffuseIndex != -1) {
        color = texture(texSampler[material.diffuseIndex], fs_in.UV);
    } else {
        color = material.albedoColor;
    }
    COLOR = color;
}