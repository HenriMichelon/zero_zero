#version 450

layout(set = 0, binding = 1) uniform samplerCube skybox;
layout (location = 0) in vec3 UV;
layout (location = 1) in vec4 AMBIENT;
layout (location = 0) out vec4 COLOR;

void main() {
    vec4 color = texture(skybox, vec3(UV.x, UV.y, -UV.z));
//    COLOR =  vec4(AMBIENT.w * AMBIENT.rgb * color.rgb, 1.0);
    COLOR =  vec4(color.rgb, 1.0);
    //COLOR = vec4(UV.x, UV.y, -UV.z, 1.0);
}