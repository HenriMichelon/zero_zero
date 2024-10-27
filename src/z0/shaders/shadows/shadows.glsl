
float shadowFactor(Light light, int cascadeIndex, vec4 fragPos) {
    // https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
    const vec3 texelSize = 1.0 / textureSize(shadowMaps[light.mapIndex], 0);
    const vec4 ShadowCoord = light.lightSpace[cascadeIndex] * fragPos;
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

float shadowFactorCubemap(Light light, vec3 fragPos) {
    // https://learnopengl.com/Advanced-Lighting/Shadows/Point-Shadows

    const float farPlane =light.farPlane;
    // get vector between fragment position and light position
    const vec3 fragToLight = fragPos - light.position;
    // use the light to fragment vector to sample from the depth map
    float closestDepth = texture(shadowMapsCubemap[light.mapIndex], fragToLight).r;
    // it is currently in linear range between [0,1]. Re-transform back to original value
    closestDepth *= farPlane; // far plane
    const float currentDepth = length(fragToLight);

    //PCF
    float shadow       = 0.0;
    const float bias   = 0.15;
    const int samples  = 20;
    const float viewDistance = length(global.cameraPosition - fragPos);
    const float diskRadius   = (1.0 + (viewDistance / farPlane)) / 25.0;
    for(int i = 0; i < samples; ++i) {
        float closestDepth = texture(shadowMapsCubemap[light.mapIndex], fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= farPlane;   // undo mapping [0;1]
        shadow += (currentDepth - bias) > closestDepth ? 0.0 : 1.0;
    }
    return shadow /= float(samples);
}
