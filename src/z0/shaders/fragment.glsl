#include "input_datas.glsl"
#include "lighting.glsl"
#include "shadows.glsl"
#extension GL_EXT_debug_printf: enable

layout (location = 0) in VertexOut fs_in;
layout (location = 0) out vec4 COLOR;

vec2 uvTransform(TextureInfo texture) {
    mat3 translation = mat3(1,0,0, 0,1,0, texture.offset.x, texture.offset.y, 1);
    mat3 scale = mat3(texture.scale.x,0,0, 0,texture.scale.y,0, 0,0,1);
    return (translation * scale * vec3(fs_in.UV, 1)).xy;
}

// Converts a color from linear light gamma to sRGB gamma
//vec4 fromLinear(vec4 linearRGB) {
//    bvec3 cutoff = lessThan(linearRGB.rgb, vec3(0.0031308));
//    vec3 higher = vec3(1.055)*pow(linearRGB.rgb, vec3(1.0/2.4)) - vec3(0.055);
//    vec3 lower = linearRGB.rgb * vec3(12.92);
//    return vec4(mix(higher, lower, cutoff), linearRGB.a);
//}

// Converts a color from sRGB gamma to linear light gamma
vec4 toLinear(vec4 sRGB) {
    bvec3 cutoff = lessThan(sRGB.rgb, vec3(0.04045));
    vec3 higher = pow((sRGB.rgb + vec3(0.055))/vec3(1.055), vec3(2.4));
    vec3 lower = sRGB.rgb/vec3(12.92);
    return vec4(mix(higher, lower, cutoff), sRGB.a);
}

vec4 fragmentColor(vec4 color, bool useColor) {
    Material material = materials.material[pushConstants.materialIndex];
    Texture tex = textures.texture[pushConstants.materialIndex];
    if (!useColor) {
        // We don't use the color parameter : get the color from the material
        color = material.albedoColor;
        if (tex.diffuseTexture.index != -1) {
            // We have a texture : get the color from the texture
            color = texture(texSampler[tex.diffuseTexture.index], uvTransform(tex.diffuseTexture));
        }
    }
//    color = vec4(fs_in.UV.x, fs_in.UV.y, 1.0f, 1.0f);

    // if TRANSPARENCY_SCISSOR or TRANSPARENCY_SCISSOR_ALPHA
    // discard the fragment if the alpha value < scissor value of the material
    if (((material.transparency == TRANSPARENCY_SCISSOR) || (material.transparency == TRANSPARENCY_SCISSOR_ALPHA)) && (color.a < material.alphaScissor)) {
        discard;
    }
    const float transparency = (material.transparency == TRANSPARENCY_ALPHA || material.transparency == TRANSPARENCY_SCISSOR_ALPHA) ? color.a : 1.0f;

    vec3 normal;
    if (tex.normalTexture.index != -1) {
        // If we have a normal texture
        normal = texture(texSampler[tex.normalTexture.index], uvTransform(tex.normalTexture)).rgb * 2.0 - 1.0;
        // https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#_material_normaltextureinfo_scale
        normal = normalize(fs_in.TBN * normal * 2.0 - 1.0) * vec3(material.normalScale, material.normalScale, 1.0f);
    } else {
        // We don't have a texture, get the calculated normal
        normal = fs_in.NORMAL;
    }
//    return  vec4(normal, 1.0); // debug normals

    // Material properties
    const float metallic  = tex.metallicTexture.index == -1 ?
            material.metallicFactor :
            material.metallicFactor * texture(texSampler[tex.metallicTexture.index], uvTransform(tex.metallicTexture)).b;
    const float roughness = tex.roughnessTexture.index == -1 ?
            material.roughnessFactor :
            material.roughnessFactor * (texture(texSampler[tex.roughnessTexture.index], uvTransform(tex.roughnessTexture)).g);
    // Fresnel reflectance at normal incidence (for metals use albedo color).
    const vec3 F0 = mix(Fdielectric, color.rgb, metallic);
    // Calculate the diffuse light from the scene's lights
    vec3 diffuse = vec3(0.0f);
    for(uint i = 0; i < global.lightsCount; i++) {
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
                diffuse += factor * calcDirectionalLight(light, color.rgb, normal, metallic, roughness, fs_in.VIEW_DIRECTION, F0);
                break;
            }
            case LIGHT_SPOT: {
                if (light.mapIndex != -1) {
                    factor = shadowFactor(light, 0, fs_in.GLOBAL_POSITION);
                }
                diffuse += factor * calcPointLight(light, color.rgb, normal, metallic, roughness, fs_in.VIEW_DIRECTION, fs_in.GLOBAL_POSITION.xyz, F0);
                break;
            }
            case LIGHT_OMNI: {
                if (light.mapIndex != -1) {
                    factor = shadowFactorCubemap(light, fs_in.GLOBAL_POSITION.xyz);
                }
                diffuse += factor * calcPointLight(light, color.rgb, normal, metallic, roughness, fs_in.VIEW_DIRECTION, fs_in.GLOBAL_POSITION.xyz, F0);
                break;
            }
        }
    }

    if (tex.emissiveTexture.index != -1) {
        diffuse += material.emissiveFactor *
                   toLinear(texture(texSampler[tex.emissiveTexture.index], uvTransform(tex.emissiveTexture))).rgb *
                   material.emissiveStrength; // https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_emissive_strength/README.md
    }

    // The global ambient light, always applied
    const float ambientOcclusion = tex.ambientOcclusionTexture.index == -1 ?
            1.0f :
            texture(texSampler[tex.ambientOcclusionTexture.index], uvTransform(tex.ambientOcclusionTexture)).b;
    vec3 ambient = color.rgb;
    if (global.ambientIBL) {
        ambient = Ambient(ambient, normal, fs_in.VIEW_DIRECTION, metallic, roughness, F0);
    }
    ambient *= global.ambient.w * global.ambient.rgb * ambientOcclusion;

    return vec4(ambient + diffuse, transparency);
}