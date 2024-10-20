#version 460
#extension GL_EXT_debug_printf: enable

layout (location = 0) in vec3 position;
layout (location = 2) in vec2 uv;

layout (location = 0) out vec2 UV;
layout (location = 1) out vec4 WORLD_POSITION;

layout(push_constant) uniform PushConstants {
    mat4  lightSpace;
    mat4  model;
    vec3  lightPosition;
    float farPlane;
} pushConstants;

void main() {
    UV = uv;
    WORLD_POSITION = pushConstants.model * vec4(position, 1.0f);
    gl_Position = pushConstants.lightSpace * WORLD_POSITION;
}