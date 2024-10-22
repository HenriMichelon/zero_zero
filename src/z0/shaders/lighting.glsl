#include "pbr.glsl"

vec3 calcDirectionalLight(Light light, vec3 albedo, vec3 normal, Material material, vec3 viewDirection, vec2 uv, float shadowFactor) {
    float metallic = material.metallicFactor * texture(texSampler[material.metallicIndex], uv).b;
    float roughness = material.roughnessFactor * texture(texSampler[material.roughnessIndex], uv).g;

    // https://learnopengl.com/PBR/Theory
    vec3 N = normalize (normal);
    vec3 V = viewDirection;
    vec3 L = normalize (-light.direction);
    vec3 H = normalize (V + L);
    vec3  F0 = mix (vec3 (0.04), pow(albedo, vec3 (2.2)), metallic);

    // Cook-Torrance BRDF
    float NDF = distributionGGX(N, H, roughness);
    float G   = geometrySmith(N, V, L, roughness);
    vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);
    vec3  kD  = vec3(1.0) - F;
    kD *= 1.0 - metallic;

    vec3  numerator   = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    vec3  specular    = numerator / max(denominator, 0.0001);

    float NdotL = max(dot(N, L), 0.0);
    return light.color.rgb * light.color.w * (kD * pow(albedo, vec3 (2.2)) / PI + specular) * NdotL * shadowFactor;
//    return light.color.rgb * light.color.w * (kD * albedo / PI + specular) * NdotL *  shadowFactor;

//    const vec3 lightDir = normalize(-light.direction);
//    const float diff = max(dot(normal, lightDir), 0.0);
//    const vec3 diffuse = diff * light.color.rgb * light.color.w * color.rgb;
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

vec3 calcPointLight(Light light, vec4 color, vec3 normal, Material material, vec3 fragPos, vec3 viewDirection, vec2 uv) {
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
    return vec3(0, 0, 0);
}
