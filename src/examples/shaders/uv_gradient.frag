#version 450

#include "../../z0/shaders/fragment.glsl"

void main() {
    COLOR = fragmentColor(vec4(fs_in.UV.x, fs_in.UV.y, material.parameters[0], 0.5), true);
}