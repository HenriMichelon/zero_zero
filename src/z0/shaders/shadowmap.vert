#version 450

layout (location = 0) in vec3 position;
layout (location = 2) in vec2 uv;

layout (location = 0) out vec2 UV;
//layout (location = 1) out vec3 COL;

layout (binding = 0) uniform GlobalUBO {
    mat4 lightSpace;
} global;

layout (binding = 1) uniform ModelUBO {
    mat4 matrix;
} model;


//float rand(vec2 co) {
//    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
//}
//
//vec3 randomColor(vec2 uv) {
//    float r = rand(uv + 0.1);
//    float g = rand(uv + 0.2);
//    float b = rand(uv + 0.3);
//    return vec3(r, g, b);
//}


void main() {
    UV = uv;
    //    COL = randomColor(uv);
    gl_Position = global.lightSpace * model.matrix * vec4(position, 1.0);
}