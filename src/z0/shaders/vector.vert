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
    vec2 pos = {position.x, -position.y};
    gl_Position = vec4(pos, 0, 1);
}
