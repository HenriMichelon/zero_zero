#version 450 core
layout(location = 0) out vec4 COLOR;

layout(set = 0, binding = 0) uniform CommandUniformBuffer  {
    vec4    color;
    int     textureIndex;
    float   clipX;
    float   clipY;
} command;

layout(set = 0, binding = 1) uniform sampler2D texSampler[100]; // VectorRenderer.MAX_IMAGES

layout(location = 0) in vec2 UV;

void main()  {
    if (command.textureIndex == -1) {
        COLOR = command.color;
    } else {
        if ((UV.x > command.clipX) || (UV.y < command.clipY)) { 
            discard;
        }
        COLOR = command.color * texture(texSampler[command.textureIndex], UV);
    }
}
