#version 450

layout (location = 0) in vec2 UV;
layout (location = 1) in vec4 WORLD_POSITION;

layout (location = 0) out vec4 COLOR;

layout(push_constant) uniform PushConstants {
    mat4  lightSpace;
    mat4  model;
    vec3  lightPosition;
    float farPlane;
} pushConstants;

void main() {
    // TODO transparency
    gl_FragDepth = length(WORLD_POSITION.xyz - pushConstants.lightPosition) / pushConstants.farPlane;
}
