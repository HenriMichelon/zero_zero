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
    if (((material.transparency == 2) || (material.transparency == 3)) && (color.a < material.alphaScissor)) {
        discard;
    }

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

    // The global ambient light, always applied
    vec3 ambient = global.ambient.w * global.ambient.rgb * color.rgb;

    // Compute the diffuse light from the scene's lights
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
                diffuse += calcDirectionalLight(light, color.rgb, normal, material, fs_in.VIEW_DIRECTION, fs_in.UV, factor);
                break;
            }
            case LIGHT_SPOT: {
                if (light.mapIndex != -1) {
                    factor = shadowFactor(light, 0, fs_in.GLOBAL_POSITION);
                }
                diffuse += factor * calcPointLight(light, color, normal, material, fs_in.GLOBAL_POSITION.xyz, fs_in.VIEW_DIRECTION, fs_in.UV);
                break;
            }
            case LIGHT_OMNI: {
                if (light.mapIndex != -1) {
                    factor = shadowFactorCubemap(light, fs_in.GLOBAL_POSITION.xyz);
                }
                diffuse += factor * calcPointLight(light, color, normal, material, fs_in.GLOBAL_POSITION.xyz, fs_in.VIEW_DIRECTION, fs_in.UV);
                break;
            }
        }
    }

    diffuse = ambient + diffuse;

    diffuse = diffuse / (diffuse + vec3(1.0));
    diffuse = pow(diffuse, vec3(1.0/2.2));

    return vec4(diffuse, material.transparency == 1 || material.transparency == 3 ? color.a : 1.0);
}