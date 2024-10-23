#include "pbr.glsl"

vec3 calcPBRLight(Light light, vec3 albedo, vec3 N, vec3 L, float attenuation, float metallic, float roughness, vec3 V) {
    // https://learnopengl.com/PBR/Theory
    const vec3 radiance = light.color.rgb * light.color.w * attenuation;
    const vec3 H = normalize(V + L);
    const vec3 F0 = mix (vec3 (0.04), albedo, metallic);

    // Cook-Torrance BRDF
    const float NDF = distributionGGX(N, H, roughness);
    const float G   = geometrySmith(N, V, L, roughness);
    const vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);
    vec3  kD  = vec3(1.0) - F;
    kD *= 1.0 - metallic;

    const vec3  numerator   = NDF * G * F;
    const float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    const vec3  specular    = numerator / denominator;

    const float NdotL = max(dot(N, L), 0.0);
    return radiance * (kD * albedo / PI + specular) * NdotL;
}

vec3 calcDirectionalLight(Light light, vec3 albedo, vec3 normal, float metallic, float roughness, vec3 viewDirection) {
    return calcPBRLight(light, albedo, normal, -light.direction, 1.0f, metallic, roughness, viewDirection);
//    const vec3 lightDir = normalize(-light.direction);
//    const float diff = max(dot(normal, lightDir), 0.0);
//    const vec3 diffuse = diff * light.color.rgb * light.color.w * albedo.rgb;
//    if (material.specularIndex != -1) {
//        // Blinn-Phong
//        // https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
//        const vec3 halfwayDir = normalize(lightDir + viewDirection);
//        const float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0*3);
//        const vec3 specular = light.specular * spec * light.color.rgb * texture(texSampler[material.specularIndex], uv).rgb;
//        return diffuse + specular;
//    }
//    return diffuse;
}

vec3 calcPointLight(Light light, vec3 albedo, vec3 normal,float metallic, float roughness, vec3 viewDirection, vec3 fragPos) {
    const float attenuation = clamp(1.0 - length(light.position - fragPos)/light.range, 0.0, 1.0);
    const vec3 lightDir = normalize(light.position - fragPos);
    const vec3 diffuse = calcPBRLight(light, albedo, normal, lightDir, attenuation, metallic, roughness, viewDirection);
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
        return intensity * diff * diffuse;
    }
    return vec3(0.0f);
/*
    const float dist = length(light.position - fragPos);
    const float attenuation = clamp(1.0 - dist/light.range, 0.0, 1.0);
    const vec3 lightDir = normalize(light.position - fragPos);
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
            const vec3 halfwayDir = normalize(lightDir + viewDirection);
            const float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0*3);
            const vec3 specular = intensity * attenuation * light.specular * spec * light.color.rgb * texture(texSampler[material.specularIndex], uv).rgb;
            return diffuse + specular;
        }
        return diffuse;
    }
    return vec3(0, 0, 0);*/
}
