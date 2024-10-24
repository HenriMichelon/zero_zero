#include "input_datas.glsl"
#include "lighting.glsl"
#include "shadows.glsl"
#extension GL_EXT_debug_printf: enable

layout (location = 0) in VertexOut fs_in;
layout (location = 0) out vec4 COLOR;

vec4 fragmentColor(vec4 color, bool useColor) {
    Material material = materials.material[pushConstants.materialIndex];
    if (!useColor) {
        // We don't use the color parameter : get the color from the material
        color = material.albedoColor;
        if (material.diffuseIndex != -1) {
            // We have a texture : get the color from the texture
            color = color * texture(texSampler[material.diffuseIndex], fs_in.UV);
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
    if (material.normalIndex != -1) {
        // If we have a normal texture
        normal = texture(texSampler[material.normalIndex], fs_in.UV).rgb * 2.0 - 1.0;
        normal = normalize(fs_in.TBN * normal);
    } else {
        // We don't have a texture, get the calculated normal
        normal = fs_in.NORMAL;
    }
//    return  vec4(normal, 1.0); // debug normals

    // Material properties
    const float metallic  = material.metallicIndex == -1 ? 0.0f : material.metallicFactor * texture(texSampler[material.metallicIndex], fs_in.UV).b;
    const float roughness = material.roughnessIndex == -1 ? 1.0f : material.roughnessFactor * (texture(texSampler[material.roughnessIndex], fs_in.UV).g);
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

    if (material.emissiveIndex != -1) {
        diffuse += material.emissiveFactor * texture(texSampler[material.emissiveIndex], fs_in.UV).rgb;
    }

    // The global ambient light, always applied
    const float ambientOcclusion  = material.ambientOcclusionIndex == -1 ? 1.0f : texture(texSampler[material.ambientOcclusionIndex], fs_in.UV).b;
    const vec3 ambient = global.ambient.w * global.ambient.rgb * color.rgb * ambientOcclusion;

    return vec4(ambient + diffuse, transparency);
}