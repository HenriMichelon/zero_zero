/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 450
#include "utils.glsl"

layout (location = 0) in vec3 NORMAL;
layout (location = 1) in vec2 UV;
layout (location = 2) in mat3 TBN;

layout (location = 0) out vec4 COLOR;

void main() {
    const Material material = materials.material[pushConstants.materialIndex];
    const Texture textures = textures.texture[pushConstants.materialIndex];
    vec3 normal;
    if (textures.normalTexture.index != -1) {
        normal = normalize(2.0f * texture(texSampler[textures.normalTexture.index], uvTransform(textures.normalTexture, UV)).rgb - 1.0f);
        normal = normalize(TBN * normal) * vec3(material.normalScale, material.normalScale, 1.0f);
    } else {
        normal =  normalize(NORMAL);
    }
    COLOR = vec4(normal * 0.5 + 0.5, 0.0f);
}
