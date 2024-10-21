#include "input_datas.glsl"
#extension GL_EXT_debug_printf: enable

layout (location = 0) in VertexOut fs_in;
layout (location = 0) out vec4 COLOR;

vec3 calcDirectionalLight(DirectionalLight light, vec4 color, vec3 normal, Material material) {
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color.rgb * light.color.w * color.rgb;
    if (material.specularIndex != -1) {
        // Blinn-Phong
        // https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
        vec3 halfwayDir = normalize(lightDir + fs_in.VIEW_DIRECTION);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess*3);
        vec3 specular = light.specular * spec * light.color.rgb * texture(texSampler[material.specularIndex], fs_in.UV).rgb;
        return diffuse + specular;
    }
    return diffuse;
}

vec3 calcPointLight(PointLight light, vec4 color, vec3 normal, Material material) {
    float dist = length(light.position - fs_in.GLOBAL_POSITION.xyz);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));
    vec3 lightDir = normalize(light.position - fs_in.GLOBAL_POSITION.xyz);
    float intensity = 1.0f;
    bool cutOff = light.isSpot;

    if (cutOff) {
        float theta = dot(lightDir, normalize(-light.direction));
        float epsilon = light.cutOff - light.outerCutOff;
        intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
        cutOff = theta <= light.outerCutOff;
    }

    if (!cutOff) {
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = intensity * attenuation * diff * light.color.rgb * light.color.w * color.rgb;
        if (material.specularIndex != -1) {
            vec3 halfwayDir = normalize(lightDir + fs_in.VIEW_DIRECTION);
            float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess*3);
            vec3 specular = intensity * attenuation * light.specular * spec * light.color.rgb * texture(texSampler[material.specularIndex], fs_in.UV).rgb;
            return diffuse + specular;
        }
        return diffuse;
    }
    return vec3(0, 0, 0);
}

// https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
float shadowFactor(int shadowMapIndex, int cascadeIndex) {
    vec3 texelSize = 1.0 / textureSize(shadowMaps[shadowMapIndex], 0);

    vec4 ShadowCoord = shadowMapsInfos.shadowMaps[shadowMapIndex].lightSpace[cascadeIndex] * fs_in.GLOBAL_POSITION;
    vec3 projCoords = ShadowCoord.xyz / ShadowCoord.w;
    if (projCoords.z > 1.0) return 1.0f;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    const bool outOfView = (projCoords.x < 0.001f || projCoords.x > 0.999f || projCoords.y < 0.001f || projCoords.y > 0.999f);
    if (outOfView) return 1.0f;
    float closestDepth = texture(shadowMaps[shadowMapIndex], vec3(projCoords.xy, float(cascadeIndex))).r;
    float currentDepth = projCoords.z;

    const float bias = 0.0005;
    float shadow = 0.0;
    for(int x = -1; x <= 1; ++x)  {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMaps[shadowMapIndex], vec3(projCoords.xy + vec2(x, y) * texelSize.xy, float(cascadeIndex))).r;
            shadow += (currentDepth - bias) > pcfDepth ? 0.0 : 1.0;
        }
    }
    return shadow /= 9.0;
}
int getCubemapFaceIndex(vec3 direction) {
    vec3 absDir = abs(direction);

    // Find the largest component (which axis to project onto)
    if (absDir.x > absDir.y && absDir.x > absDir.z) {
        return direction.x > 0.0 ? 0 : 1; // Positive X or Negative X
    } else if (absDir.y > absDir.x && absDir.y > absDir.z) {
        return direction.y > 0.0 ? 2 : 3; // Positive Y or Negative Y
    } else {
        return direction.z > 0.0 ? 5 : 4; // Positive Z or Negative Z
    }
}

float shadowFactorCubemap(int shadowMapIndex) {
    // get vector between fragment position and light position
    vec3 fragToLight = fs_in.GLOBAL_POSITION.xyz - shadowMapsInfos.shadowMaps[shadowMapIndex].lightPosition;
    // use the light to fragment vector to sample from the depth map
    float closestDepth = texture(shadowMapsCubemap[shadowMapIndex], fragToLight).r;
    // it is currently in linear range between [0,1]. Re-transform back to original value
    closestDepth *= 100.0f; // far plane
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // now test for shadows
    float bias = 0.05;
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}

float shadowFactorCubemap(int shadowMapIndex) {
    // get vector between fragment position and light position
    vec3 pos = fs_in.GLOBAL_POSITION.xyz;
    vec3 fragToLight = pos - shadowMapsInfos.shadowMaps[shadowMapIndex].lightPosition;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // use the light to fragment vector to sample from the depth map
    float closestDepth = texture(shadowMapsCubemap[shadowMapIndex], fragToLight).r;
    // it is currently in linear range between [0,1]. Re-transform back to original value
    closestDepth *= 100.0f; // far plane
    const float bias = 0.005;
    float shadow = currentDepth -  bias > closestDepth ? 0.0 : 1.0;
    return shadow;
}

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
    vec3 ambient = global.ambient.w * global.ambient.rgb;

    // Compute the diffuse light from the scene's lights
    vec3 diffuse = vec3(0, 0, 0);
    if (global.haveDirectionalLight) {
        // We currently support only one directional light
        diffuse = calcDirectionalLight(global.directionalLight, color, normal, material);
    }
    // Acculumate all other lights (spots & omnis)
    for(int i = 0; i < global.pointLightsCount; i++) {
        diffuse += calcPointLight(pointLights.lights[i], color, normal, material);
    }

    // Compute the shadow factor, default to no shadow if not activated
    // The shadow factor decrease the diffuse light
    // 1.0 -> no effect, 0.0 -> no diffuse light (only ambient light)
    float shadow = 1.0f;
    if (global.shadowMapsCount > 0) {
        if (global.cascadedShadowMapIndex != -1) {
            // We have a cascaded shadow map,
            // get cascade index maps for the current fragment's view Z position
            int cascadeIndex = 0;
            for (int index = 0; index < global.cascadesCount; index++) {
                  if (fs_in.CLIPSPACE_Z > shadowMapsInfos.shadowMaps[global.cascadedShadowMapIndex].cascadeSplitDepth[index]) {
                      cascadeIndex = index + 1;
                  }
            }
            shadow = 0.0f;
            // Get the shadow factor for the cascade
            float factor = shadowFactor(global.cascadedShadowMapIndex, cascadeIndex);
            // If no shadow try the next cascades (for objets behind the light position in the cascade)
            if ((factor >= 0.1f) && (cascadeIndex < global.cascadesCount)) {
                float nextFactor = shadowFactor(global.cascadedShadowMapIndex, cascadeIndex +1);
                if (nextFactor < 0.1f) factor = nextFactor;
            }
            if ((factor >= 0.1f) && ((cascadeIndex+1) < global.cascadesCount)) {
                float nextFactor = shadowFactor(global.cascadedShadowMapIndex, cascadeIndex +2);
                if (nextFactor < 0.1f) factor = nextFactor;
            }
            shadow = factor;

            // Accumulate the shadow factor for the others lights
            for (int i = 0; i < global.shadowMapsCount; i++) {
                if (i != global.cascadedShadowMapIndex) shadow += shadowFactor(i, 0);
            }
            // Display the cascade splits for debug
//            switch(cascadeIndex) {
//                case 0 :
//                    color.rgb *= vec3(1.0f, 0.25f, 0.25f);
//                    break;
//                case 1 :
//                    color.rgb *= vec3(0.25f, 1.0f, 0.25f);
//                    break;
//                case 2 :
//                    color.rgb *= vec3(0.25f, 0.25f, 1.0f);
//                    break;
//                case 3 :
//                    color.rgb *= vec3(1.0f, 1.0f, 0.25f);
//                    break;
//            }
        } else {
            // We don't have any cascaded shadow map,
            // accumulate the shadow factor of all maps
            shadow = 0.0f;
            for (int i = 0; i < global.shadowMapsCount; i++) {
                if (shadowMapsInfos.shadowMaps[i].isCubemap) {
                    shadow += shadowFactorCubemap(i);
                } else {
                    shadow += shadowFactor(i, 0);
                }
            }
        }
    }

    return vec4((ambient + shadow * diffuse) * color.rgb, material.transparency == 1 || material.transparency == 3 ? color.a : 1.0);
}