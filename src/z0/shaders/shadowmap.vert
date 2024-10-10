#version 450

layout (location = 0) in vec3 position;
layout (location = 2) in vec2 uv;

layout (location = 0) out vec2 UV;
//layout (location = 1) out vec3 COL;

layout(push_constant) uniform PushConstants {
    mat4 lightSpace;
    mat4 matrix;
} pushConstants;

void main() {
    UV = uv;
    gl_Position = pushConstants.lightSpace * pushConstants.matrix * vec4(position, 1.0);
}