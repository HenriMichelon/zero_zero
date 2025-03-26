/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 450

#include "utils.glsl"

layout (location = 0) in VertexOut fs_in;
layout (location = 0) out vec4 COLOR;

float stepmix(const float edge0, const float edge1, const float E, const float x)  {
    float T = clamp(0.5 * (x - edge0 + E) / E, 0.0, 1.0);
    return mix(edge0, edge1, T);
}

vec3 calcPointLight(Light light, vec3 albedo, vec3 normal,float metallic, float roughness, vec3 viewDirection, vec3 fragPos, vec3 F0, const float cosLo, const float alphaSq, const float alphaDirectLighting) {
    const float attenuation = clamp(1.0 - length(light.position - fragPos)/light.range, 0.0, 1.0);
    const vec3 lightDir = normalize(light.position - fragPos);
    const vec3 diffuse = light.color.rgb * light.color.w * attenuation;
    float intensity = 1.0f;
    if (light.type == LIGHT_SPOT) {
        const float theta = dot(lightDir, normalize(-light.direction));
        const float epsilon = light.cutOff - light.outerCutOff;
        const float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
        if (theta <= light.outerCutOff) return vec3(0.0f);
        return min(intensity * diffuse, vec3(1.0f));
    } else {
        return min(diffuse, vec3(1.0f));
    }
}


#define SHADOW_FACTOR 0.2f

float shadowFactor(Light light, int cascadeIndex, vec4 fragPos) {
    const vec3 texelSize = 1.0 / textureSize(shadowMaps[light.mapIndex], 0);
    const vec4 ShadowCoord = light.lightSpace[cascadeIndex] * fragPos;
    vec3 projCoords = ShadowCoord.xyz / ShadowCoord.w;
    if (projCoords.z > 1.0) return 1.0f;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    const bool outOfView = (projCoords.x < 0.001f || projCoords.x > 0.999f || projCoords.y < 0.001f || projCoords.y > 0.999f);
    if (outOfView) return 1.0f;
    const float closestDepth = texture(shadowMaps[light.mapIndex], vec3(projCoords.xy, float(cascadeIndex))).r;
    const float currentDepth = projCoords.z;
    float bias = 0.005;
    return currentDepth - bias > closestDepth ? SHADOW_FACTOR : 1.0;
}

float shadowFactorCubemap(Light light, vec3 fragPos) {
    const float bias         = 0.15;
    const vec3 fragToLight = fragPos - light.position;
    const float currentDepth = length(fragToLight);
    const float closestDepth = texture(shadowMapsCubemap[light.mapIndex], fragToLight).r * light.farPlane;
    return currentDepth - bias > closestDepth ? SHADOW_FACTOR : 1.0;
}

void main() {
    const Material material = materials.material[pushConstants.materialIndex];
    const Texture textures = textures.texture[pushConstants.materialIndex];

    vec4 color = material.albedoColor;
    if (textures.diffuseTexture.index != -1) {
        color *= texture(texSampler[textures.diffuseTexture.index], uvTransform(textures.diffuseTexture, fs_in.UV));
    }
    if (((material.transparency == TRANSPARENCY_SCISSOR) || (material.transparency == TRANSPARENCY_SCISSOR_ALPHA)) && (color.a < material.alphaScissor)) {
        discard;
    }
    const float transparency = (material.transparency == TRANSPARENCY_ALPHA || material.transparency == TRANSPARENCY_SCISSOR_ALPHA) ? color.a : 1.0f;

    vec3 normal;
    if (textures.normalTexture.index != -1) {
        normal = normalize(2.0f * texture(texSampler[textures.normalTexture.index], uvTransform(textures.normalTexture, fs_in.UV)).rgb - 1.0f);
        normal = normalize(fs_in.TBN * normal) * vec3(material.normalScale, material.normalScale, 1.0f);
    } else {
        normal = fs_in.NORMAL;
    }

    float specular;
    float glossiness;
    if (material.metallicFactor != -1) {
        float metallic = textures.metallicTexture.index == -1 ?
            material.metallicFactor :
            material.metallicFactor * texture(texSampler[textures.metallicTexture.index], uvTransform(textures.metallicTexture, fs_in.UV)).b;
        float roughness = textures.roughnessTexture.index == -1 ?
            material.roughnessFactor :
            material.roughnessFactor * (texture(texSampler[textures.roughnessTexture.index], uvTransform(textures.roughnessTexture, fs_in.UV)).g);
        specular = mix(0.04, 1.0, metallic);
        glossiness = 1.0 - roughness;
    } else {
        specular = 1.0;
        glossiness = 1.0;
    }

    vec3 diffuseSpecular = vec3(0.0f);
    for (uint i = 0; i < global.lightsCount; i++) {
        Light light = lights.light[i];
        vec3 lightDirection;
        float shadow = 1.0f;
        if (light.type == LIGHT_DIRECTIONAL) {
            lightDirection = normalize(-light.direction);
        } else {
            if (distance(fs_in.GLOBAL_POSITION.xyz, light.position) > light.range) {
                continue;
            }
            lightDirection = normalize(light.position - fs_in.GLOBAL_POSITION.xyz);
        }

        // https://prideout.net/blog/old/blog/index.html@p=22.html
        const vec3 H = normalize(lightDirection + fs_in.VIEW_DIRECTION);
        float df = max(0.0, dot(normal, lightDirection));
        float sf = max(0.0, dot(normal, H));
        sf = pow(sf, glossiness);

        const float A = 0.1;
        const float B = 0.3;
        const float C = 0.7;
        const float D = 1.0;
        float E = fwidth(df);

        if      (df > A - E && df < A + E) df = stepmix(A, B, E, df);
        else if (df > B - E && df < B + E) df = stepmix(B, C, E, df);
        else if (df > C - E && df < C + E) df = stepmix(C, D, E, df);
        else if (df < A) df = 0.0;
        else if (df < B) df = B;
        else if (df < C) df = C;
        else df = D;

        E = fwidth(sf);
        if (sf > 0.5 - E && sf < 0.5 + E)  {
            sf = smoothstep(0.5 - E, 0.5 + E, sf);
        }
        else {
            sf = step(0.5, sf);
        }

        float diffuseIntensity  = max(dot(normal, lightDirection), 0.0);
        if (diffuseIntensity > 0) {
            vec3 diffuseColor;
            vec3 specularColor;
            if (light.type == LIGHT_DIRECTIONAL) {
                if (light.mapIndex != -1) {
                    int cascadeIndex = 0;
                    for (int index = 0; index < light.cascadesCount; index++) {
                        if (fs_in.CLIPSPACE_Z > light.cascadeSplitDepth[index]) {
                            cascadeIndex = index + 1;
                        }
                    }
                    shadow = shadowFactor(light, cascadeIndex, fs_in.GLOBAL_POSITION);
                    if ((shadow >= 0.1f) && (cascadeIndex < light.cascadesCount)) {
                        float nextFactor = shadowFactor(light, cascadeIndex +1, fs_in.GLOBAL_POSITION);
                        if (nextFactor < 0.1f) shadow = nextFactor;
                    }
                    if ((shadow >= 0.1f) && ((cascadeIndex+1) < light.cascadesCount)) {
                        float nextFactor = shadowFactor(light, cascadeIndex +2, fs_in.GLOBAL_POSITION);
                        if (nextFactor < 0.1f) shadow = nextFactor;
                    }
                }
                diffuseColor = color.rgb * light.color.rgb * light.color.w * df * shadow;
                specularColor = specular * color.rgb * sf * shadow;
            } else if (light.type == LIGHT_OMNI) {
                if (light.mapIndex != -1) {
                    shadow = shadowFactorCubemap(light, fs_in.GLOBAL_POSITION.xyz);
                }
                float attenuation = clamp(1.0 - length(light.position - fs_in.GLOBAL_POSITION.xyz)/light.range, 0.0, 1.0);
                diffuseColor = color.rgb * light.color.rgb * light.color.w * attenuation * df * shadow;
                specularColor = specular * color.rgb * attenuation * sf * shadow;
            } else {
                const float theta = dot(lightDirection, normalize(-light.direction));
                if (theta <= light.outerCutOff) {
                    continue;
                } else {
                    if (light.mapIndex != -1) {
                        shadow = shadowFactor(light, 0, fs_in.GLOBAL_POSITION);
                    }
                    float attenuation = clamp(1.0 - length(light.position - fs_in.GLOBAL_POSITION.xyz)/light.range, 0.0, 1.0);
                    const float epsilon = light.cutOff - light.outerCutOff;
                    const float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
                    diffuseColor = min(intensity *
                            color.rgb * light.color.rgb * light.color.w * attenuation * df * shadow,
                            vec3(1.0));
                    specularColor = min(intensity *
                            specular * color.rgb * attenuation * sf * shadow,
                            vec3(1.0));
                }
            }
            diffuseSpecular = clamp(diffuseSpecular + diffuseColor + specularColor, 0.0, 1.0);
        }

    }
    const vec3 ambient = clamp(color.rgb * global.ambient.w * global.ambient.rgb, 0.0, 1.0);
    vec3 emmissiveColor = material.emissiveFactor;
    if (textures.emissiveTexture.index != -1) {
        emmissiveColor *= toLinear(texture(texSampler[textures.emissiveTexture.index], uvTransform(textures.emissiveTexture, fs_in.UV))).rgb;
    }
    diffuseSpecular += emmissiveColor * material.emissiveStrength;
    COLOR = vec4(ambient + diffuseSpecular, transparency);
}
