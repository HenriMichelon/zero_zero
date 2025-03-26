/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 450
#include "utils.glsl"

layout (location = 0) out vec4 COLOR;

layout (location = 0) in vec2 UV;

void main() {
    const Material material = materials.material[pushConstants.materialIndex];
    const Texture textures = textures.texture[pushConstants.materialIndex];
    vec4 color = material.albedoColor;
    if (textures.diffuseTexture.index != -1) {
        color = texture(texSampler[textures.diffuseTexture.index], uvTransform(textures.diffuseTexture, UV));
    }
    COLOR = vec4(color.rgb, 1.0);
}
