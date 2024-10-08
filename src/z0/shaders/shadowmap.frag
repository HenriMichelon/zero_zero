#version 450

layout (location = 0) in vec2 UV;
//layout (location = 1) in vec3 COL;
layout (location = 0) out vec4 COLOR;

void main() {
//    COLOR = vec4(COL, 1.0f); //vec4(UV.x, UV.y, 1.0, 1.0);
    COLOR = vec4(UV.x, UV.y, 1.0, 1.0);
}
