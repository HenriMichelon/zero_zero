#include "input_datas.glsl"
#extension GL_EXT_debug_printf: enable

layout (location = 0) in VertexOut fs_in;
layout (location = 0) out vec4 COLOR;

vec3 calcDirectionalLight(Light light, vec4 color, vec3 normal, Material material) {
    const vec3 lightDir = normalize(-light.direction);
    const float diff = max(dot(normal, lightDir), 0.0);
    const vec3 diffuse = diff * light.color.rgb * light.color.w * color.rgb;
    if (material.specularIndex != -1) {
        // Blinn-Phong
        // https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
        const vec3 halfwayDir = normalize(lightDir + fs_in.VIEW_DIRECTION);
        const float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess*3);
        const vec3 specular = light.specular * spec * light.color.rgb * texture(texSampler[material.specularIndex], fs_in.UV).rgb;
        return diffuse + specular;
    }
    return diffuse;
}

vec3 calcPointLight(Light light, vec4 color, vec3 normal, Material material) {
    const float dist = length(light.position - fs_in.GLOBAL_POSITION.xyz);
    const float attenuation = clamp(1.0 - dist/light.range, 0.0, 1.0);
    const vec3 lightDir = normalize(light.position - fs_in.GLOBAL_POSITION.xyz);
    float intensity = 1.0f;
    bool cutOff = light.type == LIGHT_SPOT;

    if (cutOff) {
        const float theta = dot(lightDir, normalize(-light.direction));
        const float epsilon = light.cutOff - light.outerCutOff;
        intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
        cutOff = theta <= light.outerCutOff;
    }

    if (!cutOff) {
        const float diff = max(dot(normal, lightDir), 0.0);
        const vec3 diffuse = intensity * attenuation * diff * light.color.rgb * light.color.w * color.rgb;
        if (material.specularIndex != -1) {
            const vec3 halfwayDir = normalize(lightDir + fs_in.VIEW_DIRECTION);
            const float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess*3);
            const vec3 specular = intensity * attenuation * light.specular * spec * light.color.rgb * texture(texSampler[material.specularIndex], fs_in.UV).rgb;
            return diffuse + specular;
        }
        return diffuse;
    }
    return vec3(0, 0, 0);
}

float shadowFactor(Light light, int cascadeIndex) {
    // https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
    const vec3 texelSize = 1.0 / textureSize(shadowMaps[light.mapIndex], 0);
    const vec4 ShadowCoord = light.lightSpace[cascadeIndex] * fs_in.GLOBAL_POSITION;
    vec3 projCoords = ShadowCoord.xyz / ShadowCoord.w;
    if (projCoords.z > 1.0) return 1.0f;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    const bool outOfView = (projCoords.x < 0.001f || projCoords.x > 0.999f || projCoords.y < 0.001f || projCoords.y > 0.999f);
    if (outOfView) return 1.0f;
    const float closestDepth = texture(shadowMaps[light.mapIndex], vec3(projCoords.xy, float(cascadeIndex))).r;
    const float currentDepth = projCoords.z;
    // PCF
    const float bias   = 0.0005;
    float shadow       = 0.0;
    for(int x = -1; x <= 1; ++x)  {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMaps[light.mapIndex], vec3(projCoords.xy + vec2(x, y) * texelSize.xy, float(cascadeIndex))).r;
            shadow += (currentDepth - bias) > pcfDepth ? 0.0 : 1.0;
        }
    }
    return shadow /= 9.0;
}

const vec3 sampleOffsetDirections[20] = vec3[] (
    vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
    vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
    vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
    vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
    vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

float shadowFactorCubemap(Light light) {
    // https://learnopengl.com/Advanced-Lighting/Shadows/Point-Shadows

    const float farPlane =light.farPlane;
    // get vector between fragment position and light position
    const vec3 fragToLight = fs_in.GLOBAL_POSITION.xyz - light.position;
    // use the light to fragment vector to sample from the depth map
    float closestDepth = texture(shadowMapsCubemap[light.mapIndex], fragToLight).r;
    // it is currently in linear range between [0,1]. Re-transform back to original value
    closestDepth *= farPlane; // far plane
    const float currentDepth = length(fragToLight);

    //PCF
    float shadow       = 0.0;
    const float bias   = 0.15;
    const int samples  = 20;
    const float viewDistance = length(global.cameraPosition - fs_in.GLOBAL_POSITION.xyz);
    const float diskRadius   = (1.0 + (viewDistance / farPlane)) / 25.0;
    for(int i = 0; i < samples; ++i) {
        float closestDepth = texture(shadowMapsCubemap[light.mapIndex], fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= farPlane;   // undo mapping [0;1]
        shadow += (currentDepth - bias) > closestDepth ? 0.0 : 1.0;
    }
    return shadow /= float(samples);
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
                    factor = shadowFactor(light, cascadeIndex);
                    // If no shadow try the next cascades (for objets behind the light position in the cascade)
                    if ((factor >= 0.1f) && (cascadeIndex < light.cascadesCount)) {
                        float nextFactor = shadowFactor(light, cascadeIndex +1);
                        if (nextFactor < 0.1f) factor = nextFactor;
                    }
                    if ((factor >= 0.1f) && ((cascadeIndex+1) < light.cascadesCount)) {
                        float nextFactor = shadowFactor(light, cascadeIndex +2);
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
                diffuse += factor * calcDirectionalLight(light, color, normal, material);
                break;
            }
            case LIGHT_SPOT: {
                if (light.mapIndex != -1) {
                    factor = shadowFactor(light, 0);
                }
                diffuse += factor * calcPointLight(light, color, normal, material);
                break;
            }
            case LIGHT_OMNI: {
                if (light.mapIndex != -1) {
                    factor = shadowFactorCubemap(light);
                }
                diffuse += factor * calcPointLight(light, color, normal, material);
                break;
            }
        }
    }

    return vec4((ambient + diffuse) * color.rgb, material.transparency == 1 || material.transparency == 3 ? color.a : 1.0);
}