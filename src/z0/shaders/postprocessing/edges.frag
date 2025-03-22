/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 450

#include "postprocessing_input.glsl"

vec2 texelSize = vec2(1.0 / 1280.0, 1.0 / 720.0); // 1.0 / texture resolution
float edgeThreshold = 0.5; // Parameter to filter only the strongest edges
vec4 edgesColor = vec4(0.0, 0.0, 0.0, 1.0);

float edgeDetection(vec2 uv) {
    // Sample depth values
    float depthCenter = texture(depthBuffer, uv).r;
    float depthLeft   = texture(depthBuffer, uv - vec2(texelSize.x, 0)).r;
    float depthRight  = texture(depthBuffer, uv + vec2(texelSize.x, 0)).r;
    float depthUp     = texture(depthBuffer, uv + vec2(0, texelSize.y)).r;
    float depthDown   = texture(depthBuffer, uv - vec2(0, texelSize.y)).r;

    // Compute depth gradient
    float depthEdge = abs(depthLeft - depthCenter) +
    abs(depthRight - depthCenter) +
    abs(depthUp - depthCenter) +
    abs(depthDown - depthCenter);

    // Sample normal values
    vec3 normalCenter = texture(normalColor, uv).rgb;
    vec3 normalLeft   = texture(normalColor, uv - vec2(texelSize.x, 0)).rgb;
    vec3 normalRight  = texture(normalColor, uv + vec2(texelSize.x, 0)).rgb;
    vec3 normalUp     = texture(normalColor, uv + vec2(0, texelSize.y)).rgb;
    vec3 normalDown   = texture(normalColor, uv - vec2(0, texelSize.y)).rgb;

    // Compute normal gradient
    float normalEdge = length(normalLeft - normalCenter) +
    length(normalRight - normalCenter) +
    length(normalUp - normalCenter) +
    length(normalDown - normalCenter);

    return clamp(depthEdge + normalEdge, 0.0, 1.0);
}

void main() {
    float edge = edgeDetection(UV);
    if (edge > edgeThreshold) {
        COLOR = edgesColor;
    } else {
        COLOR = vec4(texture(inputImage, UV).rgb, 1.0);
    }
}
