/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 450

#include "postprocessing_input.glsl"

float edgeDetection(const vec2 uv, const vec2 texelSize) {
    // Sample depth values
    const float depthCenter = texture(depthBuffer, uv).r;
    const float depthLeft   = texture(depthBuffer, uv - vec2(texelSize.x, 0)).r;
    const float depthRight  = texture(depthBuffer, uv + vec2(texelSize.x, 0)).r;
    const float depthUp     = texture(depthBuffer, uv + vec2(0, texelSize.y)).r;
    const float depthDown   = texture(depthBuffer, uv - vec2(0, texelSize.y)).r;

    // Compute depth gradient
    const float depthEdge =
        abs(depthLeft - depthCenter) +
        abs(depthRight - depthCenter) +
        abs(depthUp - depthCenter) +
        abs(depthDown - depthCenter);

    // Sample normal values
    const vec3 normalCenter = texture(normalColor, uv).rgb;
    const vec3 normalLeft   = texture(normalColor, uv - vec2(texelSize.x, 0)).rgb;
    const vec3 normalRight  = texture(normalColor, uv + vec2(texelSize.x, 0)).rgb;
    const vec3 normalUp     = texture(normalColor, uv + vec2(0, texelSize.y)).rgb;
    const vec3 normalDown   = texture(normalColor, uv - vec2(0, texelSize.y)).rgb;

    // Compute normal gradient
    const float normalEdge =
        length(normalLeft - normalCenter) +
        length(normalRight - normalCenter) +
        length(normalUp - normalCenter) +
        length(normalDown - normalCenter);

    return clamp(depthEdge + normalEdge, 0.0, 1.0);
}

layout(binding = BINDING_GLOBAL_BUFFER) uniform GobalUniformBufferObject {
    float threshold;
    float lineWidth;
    vec3  color;
} global;

void main() {
    float edge = edgeDetection(UV, pushConstants.texelSize * global.lineWidth);
    if (edge > global.threshold) {
        COLOR = vec4(global.color, 1.0);
    } else {
//        COLOR = vec4(1.0f);
        COLOR = texture(inputImage, UV);
    }
}
