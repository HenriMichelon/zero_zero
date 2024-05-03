#version 450

layout(set = 0, binding = 1) uniform samplerCube skybox;
layout (location = 0) in vec3 UV;
layout (location = 0) out vec4 COLOR;

void main() {
    COLOR = texture(skybox, vec3(UV.x, UV.y, -UV.z));
    //COLOR = vec4(UV.x, UV.y, UV.z, 1.0);
}