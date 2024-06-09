#version 450

#include "input_datas.glsl"
layout (location = 0) out vec4 COLOR;

void main() {
    COLOR = vec4(1.0 * material.parameters[0], 1.0 * material.parameters[1], 1.0 * material.parameters[2],1.0);
}