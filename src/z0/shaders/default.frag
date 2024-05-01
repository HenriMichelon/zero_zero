#version 450

#include "input_datas.glsl"

layout (location = 0) in VertexOut fs_in;
layout (location = 0) out vec4 COLOR;

void main() {
    COLOR = vec4(fs_in.UV.x, fs_in.UV.y, 0.0, 1.0);
}