
vec3 calcDirectionalLight(Light light, vec4 color, vec3 normal, Material material, vec3 viewDirection, vec2 uv) {
    const vec3 lightDir = normalize(-light.direction);
    const float diff = max(dot(normal, lightDir), 0.0);
    const vec3 diffuse = diff * light.color.rgb * light.color.w * color.rgb;
    if (material.specularIndex != -1) {
        // Blinn-Phong
        // https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
        const vec3 halfwayDir = normalize(lightDir + viewDirection);
        const float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess*3);
        const vec3 specular = light.specular * spec * light.color.rgb * texture(texSampler[material.specularIndex], uv).rgb;
        return diffuse + specular;
    }
    return diffuse;
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
            const float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess*3);
            const vec3 specular = intensity * attenuation * light.specular * spec * light.color.rgb * texture(texSampler[material.specularIndex], uv).rgb;
            return diffuse + specular;
        }
        return diffuse;
    }
    return vec3(0, 0, 0);
}
