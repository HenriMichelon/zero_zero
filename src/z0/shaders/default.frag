#version 450

#include "fragment.glsl"

void main() {
//    COLOR = vec4(fs_in.UV.x, fs_in.UV.y, 1.0, 1.0);
    vec4 color;
    COLOR = fragmentColor(color, false);
}
