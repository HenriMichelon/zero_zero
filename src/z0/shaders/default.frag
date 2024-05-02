#version 450

#include "fragment.glsl"

void main() {
    vec4 color;
    COLOR = fragmentColor(color, false);
}