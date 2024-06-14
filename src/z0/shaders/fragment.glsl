#include "input_datas.glsl"

layout (location = 0) in VertexOut fs_in;
layout (location = 0) out vec4 COLOR;

vec3 calcDirectionalLight(DirectionalLight light, vec4 color, vec3 normal) {
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

vec3 calcPointLight(PointLight light, vec4 color, vec3 normal) {
    float dist = length(light.position - fs_in.GLOBAL_POSITION.xyz);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));
    vec3 lightDir = normalize(light.position - fs_in.GLOBAL_POSITION.xyz);
    float intensity = 1.0f;
    bool cutOff = light.isSpot;

    if (light.isSpot) {
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
float shadowFactor(int shadowMapIndex) {
    vec4 ShadowCoord = shadowMapsInfos.shadowMaps[shadowMapIndex].lightSpace * fs_in.GLOBAL_POSITION;

    vec3 projCoords = ShadowCoord.xyz / ShadowCoord.w;
    if (projCoords.z > 1.0) return 1.0f;
    // Remap xy to [0.0, 1.0]
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    const bool outOfView = (projCoords.x < 0.001f || projCoords.x > 0.999f || projCoords.y < 0.001f || projCoords.y > 0.999f);
    if (outOfView) return 1.0f;

    float currentDepth = projCoords.z;
    float closestDepth = texture(shadowMaps[shadowMapIndex], projCoords.xy).r;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMaps[shadowMapIndex], 0);
    for(int x = -1; x <= 1; ++x)  {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMaps[shadowMapIndex], projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    return 1.0 - shadow;
}

vec4 fragmentColor(vec4 color, bool useColor) {
    if (!useColor) {
        if (material.diffuseIndex != -1) {
            color = texture(texSampler[material.diffuseIndex], fs_in.UV);
        } else {
            color = material.albedoColor;
        }
    }
    if (((material.transparency == 2) || (material.transparency == 3)) && (color.a < material.alphaScissor)) {
        discard;
    }
    vec3 normal;
    if (material.normalIndex != -1) {
        normal = texture(texSampler[material.normalIndex], fs_in.UV).rgb * 2.0 - 1.0;
        normal = normalize(fs_in.TBN * normal);
    } else {
        normal = fs_in.NORMAL;
    }
    //return  vec4(normal, 1.0);

    vec3 ambient = global.ambient.w * global.ambient.rgb * color.rgb;
    vec3 diffuse = vec3(0, 0, 0);
    if (global.haveDirectionalLight) {
        diffuse = calcDirectionalLight(global.directionalLight, color, normal);
    }
    vec3 result = ambient + diffuse;

    for (int i = 0; i < global.shadowMapsCount; i++) {
        float shadows = shadowFactor(i);
        result = (ambient + shadows) * result;
    }

    return vec4(result, material.transparency == 1 || material.transparency == 3 ? color.a : 1.0);
}