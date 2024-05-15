#version 450 core
layout(location = 0) out vec4 COLOR;

layout(push_constant) uniform uPushConstant {
    int textureIndex;
} pc;

layout(set = 0, binding = 0) uniform sampler2D texSampler[100]; // VectorRenderer.MAX_IMAGES

layout(location = 0) in struct {
    vec4 color;
    vec2 uv;
} In;

void main()  {
    if (pc.textureIndex == -1) {
        COLOR = In.color;
    } else {
        COLOR = In.color * texture(texSampler[pc.textureIndex], In.uv);
    }
}
