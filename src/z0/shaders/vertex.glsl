/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#include "input_datas.glsl"

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec4 tangent;

layout (location = 0) out VertexOut vs_out;

void vertexParameters(vec3 pos) {
    mat4 model = models.model[pushConstants.modelIndex];
    vs_out.POSITION = pos;
    vs_out.NORMAL = normalize(mat3(transpose(inverse(model))) * normal);
    vs_out.UV = uv;
    vs_out.GLOBAL_POSITION = model * vec4(pos, 1.0);
    vs_out.CLIPSPACE_Z = (global.view * vs_out.GLOBAL_POSITION).z;
    vs_out.VIEW_DIRECTION = normalize(global.cameraPosition - vs_out.GLOBAL_POSITION.xyz);
    gl_Position = global.projection * global.view * vs_out.GLOBAL_POSITION;
    // https://learnopengl.com/Advanced-Lighting/Normal-Mapping
    vec3 T = (vec3(model * vec4(tangent.xyz, 0.0)));
    vec3 N = (vec3(model * vec4(normal, 0.0)));
    vec3 B = cross(N, T);
    vs_out.TBN = mat3(T, B, N);
}