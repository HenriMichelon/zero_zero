#include "pbr.glsl"

vec3 Ambient(
        // base color
        const vec3 albedo,
        // Fragment normal in world space
        const vec3 N,
        // Angle between surface normal and camera direction
        const vec3 Lo,
        // Material properties
        const float metallic, float roughness,
        // Fresnel reflectance at normal incidence
        const vec3 F0,
        // Specular reflection vector.
        const float cosLo
) {
    vec3 Lr = 2.0 * cosLo * N - Lo;
    // Sample diffuse irradiance at normal direction.
    vec3 irradiance = texture(irradianceTexture, N).rgb;

    // Calculate Fresnel term for ambient lighting.
    // Since we use pre-filtered cubemap(s) and irradiance is coming from many directions
    // use cosLo instead of angle with light's half-vector (cosLh above).
    // See: https://seblagarde.wordpress.com/2011/08/17/hello-world/
    vec3 F = fresnelSchlick(F0, cosLo);

    // Get diffuse contribution factor (as with direct lighting).
    vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metallic);

    // Irradiance map contains exitant radiance assuming Lambertian BRDF, no need to scale by 1/PI here either.
    vec3 diffuseIBL = kd * albedo * irradiance;

    // Sample pre-filtered specular reflection environment at correct mipmap level.
    int specularTextureLevels = textureQueryLevels(specularTexture);
    vec3 specularIrradiance = textureLod(specularTexture, Lr, roughness * specularTextureLevels).rgb;

    // Split-sum approximation factors for Cook-Torrance specular BRDF.
    vec2 specularBRDF = texture(specularBRDF_LUT, vec2(cosLo, roughness)).rg;

    // Total specular IBL contribution.
    vec3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;

    // Total ambient lighting contribution.
    return diffuseIBL + specularIBL;
}

vec3 Radiance(const Light light,
    // base color
    const vec3 albedo,
    // Fragment normal in world space
    const vec3 N,
    // Angle between surface normal and outgoing light direction.
    const vec3 Lo,
    // Material properties
    const float metallic, float roughness,
    // Direction to light
    const vec3 Li,
    // Fresnel reflectance at normal incidence
    const vec3 F0,
    // Specular reflection vector.
    const float cosLo,
    // Light attenuation by distance
    const float attenuation
) {
    // https://learnopengl.com/PBR/Theory
    // Half-vector between Li and Lo.
    const vec3 Lh = normalize(Li + Lo);

    // Calculate angles between surface normal and various light vectors.
    const float cosLi = max(0.0, dot(N, Li));
    const float cosLh = max(0.0, dot(N, Lh));

    // Calculate Fresnel term for direct lighting.
    const vec3 F  = fresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
    // Calculate normal distribution for specular BRDF.
    const float D = ndfGGX(cosLh, roughness);
    // Calculate geometric attenuation for specular BRDF.
    const float G = gaSchlickGGX(cosLi, cosLo, alphaDirectLighting(roughness));

    // Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
    // Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
    // To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
    const vec3 kD = mix(vec3(1.0) - F, vec3(0.0), metallic);

    // Lambert diffuse BRDF.
    // We don't scale by 1/PI for lighting & material units to be more convenient.
    // See: https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
    const vec3 diffuseBRDF = kD * albedo;

    // Cook-Torrance specular microfacet BRDF.
    const vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * cosLo);

    // Total contribution for this light.
    return (diffuseBRDF + specularBRDF) * light.color.rgb * light.color.w * cosLi * attenuation;
}

vec3 calcDirectionalLight(Light light, vec3 albedo, vec3 normal, float metallic, float roughness, vec3 viewDirection, vec3 F0, const float cosLo) {
    return Radiance(light, albedo, normal, viewDirection,  metallic, roughness,  -light.direction, F0, cosLo, 1.0f);
}

vec3 calcPointLight(Light light, vec3 albedo, vec3 normal,float metallic, float roughness, vec3 viewDirection, vec3 fragPos, vec3 F0, const float cosLo) {
    const float attenuation = clamp(1.0 - length(light.position - fragPos)/light.range, 0.0, 1.0);
    const vec3 lightDir = normalize(light.position - fragPos);
    const vec3 diffuse = Radiance(light, albedo, normal, viewDirection, metallic, roughness, lightDir,F0, cosLo, attenuation);
    float intensity = 1.0f;
    if (light.type == LIGHT_SPOT) {
        const float theta = dot(lightDir, normalize(-light.direction));
        const float epsilon = light.cutOff - light.outerCutOff;
        intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
        if (theta <= light.outerCutOff) return vec3(0.0f);
    }
    return min(intensity * diffuse, vec3(1.0f));

}
