/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 450 core
layout(location = 0) out vec4 COLOR;

layout(push_constant) uniform PushConstants {
    vec4    color;
    int     textureIndex;
    float   clipX;
    float   clipY;
} command;

layout(set = 0, binding = 0) uniform sampler2D texSampler[100]; // VectorRenderer.MAX_IMAGES

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
