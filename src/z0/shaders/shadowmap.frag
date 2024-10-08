#version 450

layout (location = 0) in vec2 UV;
layout (location = 0) out vec4 COLOR;

void main() {
    COLOR = vec4(UV.x, UV.y, 0.5, 1.0);
}
