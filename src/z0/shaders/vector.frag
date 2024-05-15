#version 450 core
layout(location = 0) out vec4 COLOR;

layout(push_constant) uniform uPushConstant {
    int textureIndex;
} pc;

layout(location = 0) in struct {
    vec4 color;
    vec2 uv;
} In;

void main()  {
    COLOR = In.color;
}
