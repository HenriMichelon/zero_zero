#version 450 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 uv;

layout(location = 0) out struct {
    vec4 color;
    vec2 uv;
} Out;

void main() {
    Out.color = color;
    Out.uv = uv;
    vec2 pos = 2 * (position - 0.5); // remap to [-1,1]
    gl_Position = vec4(pos.x, -pos.y, 0, 1);
}
