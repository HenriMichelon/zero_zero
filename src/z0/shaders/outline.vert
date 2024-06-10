#version 450

#include "input_datas.glsl"

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec4 tangent;

void main() {
    vec3 scaledPosition = position * vec3(1.0f + model.outlineScale);
    vec4 globalPosition = model.matrix * vec4(scaledPosition, 1.0);
    gl_Position = global.projection * global.view * globalPosition;
}
