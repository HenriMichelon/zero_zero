#include "pbr.glsl"

vec3 calcPBRLight(Light light, vec3 albedo, vec3 N, vec3 Lo, float attenuation, float metallic, float roughness, vec3 Li) {
    // Fresnel reflectance at normal incidence (for metals use albedo color).
    vec3 F0 = mix(Fdielectric, albedo, metallic);
    // Angle between surface normal and outgoing light direction.
    float cosLo = max(0.0, dot(N, Lo));

    // https://learnopengl.com/PBR/Theory
    // Half-vector between Li and Lo.
    vec3 Lh = normalize(Li + Lo);

    // Calculate angles between surface normal and various light vectors.
    float cosLi = max(0.0, dot(N, Li));
    float cosLh = max(0.0, dot(N, Lh));

    // Calculate Fresnel term for direct lighting.
    vec3 F  = fresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
    // Calculate normal distribution for specular BRDF.
    float D = ndfGGX(cosLh, roughness);
    // Calculate geometric attenuation for specular BRDF.
    float G = gaSchlickGGX(cosLi, cosLo, roughness);

    // Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
    // Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
    // To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
    vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metallic);

    // Lambert diffuse BRDF.
    // We don't scale by 1/PI for lighting & material units to be more convenient.
    // See: https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
    vec3 diffuseBRDF = kd * albedo;

    // Cook-Torrance specular microfacet BRDF.
    vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * cosLo);

    // Total contribution for this light.
    return (diffuseBRDF + specularBRDF) * light.color.rgb * light.color.w * cosLi;
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
