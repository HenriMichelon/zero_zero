#include "pbr.glsl"

vec3 Radiance(const Light light,
              // base color
              const vec3 albedo,
              // Fragment normal in world space
              const vec3 N,
              // Angle between surface normal and outgoing light direction.
              const vec3 Lo,
              // Light attenuation by distance
              const float attenuation,
              // Material properties
              const float metallic, float roughness,
              // Direction to light
              const vec3 Li,
              // Fresnel reflectance at normal incidence
              const vec3 F0) {
    // https://learnopengl.com/PBR/Theory
    // Half-vector between Li and Lo.
    const vec3 Lh = normalize(Li + Lo);

    // Calculate angles between surface normal and various light vectors.
    const float cosLo = max(0.0, dot(N, Lo));
    const float cosLi = max(0.0, dot(N, Li));
    const float cosLh = max(0.0, dot(N, Lh));

    // Calculate Fresnel term for direct lighting.
    const vec3 F  = fresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
    // Calculate normal distribution for specular BRDF.
    const float D = ndfGGX(cosLh, roughness);
    // Calculate geometric attenuation for specular BRDF.
    const float G = gaSchlickGGX(cosLi, cosLo, roughness);

    // Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
    // Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
    // To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
    const vec3 kD = mix(vec3(1.0) - F, vec3(0.0), metallic);

    // kS is equal to Fresnel
    // vec3 kS = F;
    // For energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    // vec3 kD = vec3(1.0) - kS;
    // Multiply kD by the inverse metalness such that only non-metals
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    // kD *= 1.0 - metallic;

    // Lambert diffuse BRDF.
    // We don't scale by 1/PI for lighting & material units to be more convenient.
    // See: https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
    const vec3 diffuseBRDF = kD * albedo;

    // Cook-Torrance specular microfacet BRDF.
    const vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * cosLo);

    // Total contribution for this light.
    return (diffuseBRDF + specularBRDF) * light.color.rgb * light.color.w * cosLi;
}

vec3 calcDirectionalLight(Light light, vec3 albedo, vec3 normal, float metallic, float roughness, vec3 viewDirection, vec3 F0) {
    return Radiance(light, albedo, normal, -light.direction, 1.0f, metallic, roughness, viewDirection, F0);
}

vec3 calcPointLight(Light light, vec3 albedo, vec3 normal,float metallic, float roughness, vec3 viewDirection, vec3 fragPos, vec3 F0) {
    const float attenuation = clamp(1.0 - length(light.position - fragPos)/light.range, 0.0, 1.0);
    const vec3 lightDir = normalize(light.position - fragPos);
    const vec3 diffuse = Radiance(light, albedo, normal, lightDir, attenuation, metallic, roughness, viewDirection, F0);
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

}
