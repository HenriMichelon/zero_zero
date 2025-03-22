/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 450

#include "postprocessing_input.glsl"

layout(binding = BINDING_GLOBAL_BUFFER) uniform GobalUniformBufferObject {
    /* This parameter defines the maximum span size for searching neighboring pixels when detecting edges.
        A higher value allows smoothing of wider edges but can also introduce excessive blurring. */
    float spanMax; // 8.0;
    /* This parameter is a multiplier used to reduce the sensitivity of edge detection.
        A lower value makes the algorithm more sensitive to small details, while a higher value makes it less sensitive.*/
    float reduceMul; // 1.0 / 8.0;
    /*  This parameter sets the minimum reduction threshold for edge detection.
        This helps to avoid visual artifacts by ensuring that subtle edges are not completely ignored*/
    float reduceMin; // 1.0 / 128.0;
} global;

vec3 fxaa(sampler2D tex, const vec2 uv, const vec2 texelSize) {
//    vec2 rcpFrame = 1.0 / vec2(textureSize(tex, 0));
    vec3 rgbNW = texture(tex, uv + vec2(-1.0, -1.0) * texelSize).rgb;
    vec3 rgbNE = texture(tex, uv + vec2(1.0, -1.0) * texelSize).rgb;
    vec3 rgbSW = texture(tex, uv + vec2(-1.0, 1.0) * texelSize).rgb;
    vec3 rgbSE = texture(tex, uv + vec2(1.0, 1.0) * texelSize).rgb;
    vec3 rgbM = texture(tex, uv).rgb;

    vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM = dot(rgbM, luma);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * global.reduceMul), global.reduceMin);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2(global.spanMax, global.spanMax), max(vec2(-global.spanMax, -global.spanMax), dir * rcpDirMin)) * texelSize;

    vec3 rgbA = 0.5 * (texture(tex, uv + dir * (1.0 / 3.0 - 0.5)).rgb + texture(tex, uv + dir * (2.0 / 3.0 - 0.5)).rgb);
    vec3 rgbB = rgbA * 0.5 + 0.25 * (texture(tex, uv + dir * -0.5).rgb + texture(tex, uv + dir * 0.5).rgb);

    float lumaB = dot(rgbB, luma);
    if ((lumaB < lumaMin) || (lumaB > lumaMax)) {
        return rgbA;
    } else {
        return rgbB;
    }
}

void main() {
    COLOR = vec4(fxaa(inputImage, UV, pushConstants.texelSize), 1.0);
}
