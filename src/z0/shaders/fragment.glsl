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

vec4 fragmentColor(vec4 color, vec3 normal, bool useColor) {
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
    if (material.normalIndex != -1) {
        normal = texture(texSampler[material.normalIndex], fs_in.UV).rgb * 2.0 - 1.0;
        normal = normalize(fs_in.TBN * normal);
    } else {
        normal = fs_in.NORMAL;
    }
    //COLOR = vec4(normal, 1.0);

    vec3 ambient = global.ambient.w * global.ambient.rgb * color.rgb;
    vec3 diffuse = vec3(0, 0, 0);
    if (global.haveDirectionalLight) {
        diffuse = calcDirectionalLight(global.directionalLight, color, normal);
    }
    vec3 result = ambient + diffuse;

    return vec4(result, material.transparency == 1 || material.transparency == 3 ? color.a : 1.0);
}