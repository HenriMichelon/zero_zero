/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#include "utils.glsl"
#include "pbr/lighting.glsl"
#include "shadows/shadows.glsl"
#extension GL_EXT_debug_printf: enable

layout (location = 0) in VertexOut fs_in;
layout (location = 0) out vec4 COLOR;

vec4 fragmentColor(vec4 color, bool useColor) {
    Material material = materials.material[pushConstants.materialIndex];
    Texture tex = textures.texture[pushConstants.materialIndex];
    if (!useColor) {
        // Get the color from the material base color factor
        color = material.albedoColor;
        if (tex.diffuseTexture.index != -1) {
            // We have a texture : apply the color from the texture
            color *= texture(texSampler[tex.diffuseTexture.index], uvTransform(tex.diffuseTexture, fs_in.UV));
        }
    }
//    return color;
//    color = vec4(fs_in.UV.x, fs_in.UV.y, 1.0f, 1.0f);

    // if Transparency::SCISSOR or Transparency::SCISSOR_ALPHA
    // discard the fragment if the alpha value < scissor value of the material
    if (((material.transparency == TRANSPARENCY_SCISSOR) || (material.transparency == TRANSPARENCY_SCISSOR_ALPHA)) && (color.a < material.alphaScissor)) {
        discard;
    }
    const float transparency = (material.transparency == TRANSPARENCY_ALPHA || material.transparency == TRANSPARENCY_SCISSOR_ALPHA) ? color.a : 1.0f;

    vec3 normal;
    if (tex.normalTexture.index != -1) {
        // Get current fragment's normal and transform to world space.
        normal = normalize(2.0f * texture(texSampler[tex.normalTexture.index], uvTransform(tex.normalTexture, fs_in.UV)).rgb - 1.0f);
        // https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#_material_normaltextureinfo_scale
        normal = normalize(fs_in.TBN * normal) * vec3(material.normalScale, material.normalScale, 1.0f);
    } else {
        // We don't have a texture, get the calculated normal
        normal = fs_in.NORMAL;
    }
//    return  vec4(normal, 1.0); // debug normals

    // Material properties
    vec3 diffuse = vec3(0.0f);
    vec3 ambient = color.rgb;
    if (material.metallicFactor != -1) {
        float metallic  = tex.metallicTexture.index == -1 ?
            material.metallicFactor :
            material.metallicFactor * texture(texSampler[tex.metallicTexture.index], uvTransform(tex.metallicTexture, fs_in.UV)).b;
        float roughness = tex.roughnessTexture.index == -1 ?
            material.roughnessFactor :
            material.roughnessFactor * (texture(texSampler[tex.roughnessTexture.index], uvTransform(tex.roughnessTexture, fs_in.UV)).g);
        // Fresnel reflectance at normal incidence (for metals use albedo color).
        const vec3 F0 = mix(Fdielectric, color.rgb, metallic);
        // Specular reflection vector.
        const float cosLo = max(0.0, dot(normal, fs_in.VIEW_DIRECTION));
        // Calculate the diffuse light from the scene's lights
        for (uint i = 0; i < global.lightsCount; i++) {
            Light light = lights.light[i];
            float factor = 1.0f;
            switch (light.type) {
                case LIGHT_DIRECTIONAL: {
                    if (light.mapIndex != -1) {
                        // We have a cascaded shadow map,
                        // get cascade index maps for the current fragment's view Z position
                        int cascadeIndex = 0;
                        for (int index = 0; index < light.cascadesCount; index++) {
                            if (fs_in.CLIPSPACE_Z > light.cascadeSplitDepth[index]) {
                                cascadeIndex = index + 1;
                            }
                        }
                        // Get the shadow factor for the cascade
                        factor = shadowFactor(light, cascadeIndex, fs_in.GLOBAL_POSITION);
                        // If no shadow try the next cascades (for objets behind the light position in the cascade)
                        if ((factor >= 0.1f) && (cascadeIndex < light.cascadesCount)) {
                            float nextFactor = shadowFactor(light, cascadeIndex +1, fs_in.GLOBAL_POSITION);
                            if (nextFactor < 0.1f) factor = nextFactor;
                        }
                        if ((factor >= 0.1f) && ((cascadeIndex+1) < light.cascadesCount)) {
                            float nextFactor = shadowFactor(light, cascadeIndex +2, fs_in.GLOBAL_POSITION);
                            if (nextFactor < 0.1f) factor = nextFactor;
                        }
                        // Display the cascade splits for debug
                        //                    switch (cascadeIndex) {
                        //                        case 0 :
                        //                        color.rgb *= vec3(1.0f, 0.25f, 0.25f);
                        //                        break;
                        //                        case 1 :
                        //                        color.rgb *= vec3(0.25f, 1.0f, 0.25f);
                        //                        break;
                        //                        case 2 :
                        //                        color.rgb *= vec3(0.25f, 0.25f, 1.0f);
                        //                        break;
                        //                        case 3 :
                        //                        color.rgb *= vec3(1.0f, 1.0f, 0.25f);
                        //                        break;
                        //                    }
                    }
                    diffuse += factor * calcDirectionalLight(light, color.rgb, normal, metallic, roughness, fs_in.VIEW_DIRECTION, F0, cosLo);
                    break;
                }
                case LIGHT_SPOT: {
                    if (light.mapIndex != -1) {
                        factor = shadowFactor(light, 0, fs_in.GLOBAL_POSITION);
                    }
                    diffuse += factor * calcPointLight(light, color.rgb, normal, metallic, roughness, fs_in.VIEW_DIRECTION, fs_in.GLOBAL_POSITION.xyz, F0, cosLo);
                    break;
                }
                case LIGHT_OMNI: {
                    if (light.mapIndex != -1) {
                        factor = shadowFactorCubemap(light, fs_in.GLOBAL_POSITION.xyz);
                    }
                    diffuse += factor * calcPointLight(light, color.rgb, normal, metallic, roughness, fs_in.VIEW_DIRECTION, fs_in.GLOBAL_POSITION.xyz, F0, cosLo);
                    break;
                }
            }
        }
        if (global.ambientIBL) {
            ambient = Ambient(ambient, normal, fs_in.VIEW_DIRECTION, metallic, roughness, F0, cosLo);
        }
    }
    if (tex.emissiveTexture.index != -1) {
        diffuse += material.emissiveFactor *
        toLinear(texture(texSampler[tex.emissiveTexture.index], uvTransform(tex.emissiveTexture, fs_in.UV))).rgb *
        material.emissiveStrength;// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_emissive_strength/README.md
    }
    ambient *= global.ambient.w * global.ambient.rgb ;

    return vec4(ambient + diffuse, transparency);
}