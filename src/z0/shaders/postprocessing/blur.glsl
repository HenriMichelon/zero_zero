/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#include "postprocessing_input.glsl"

vec4 gaussianBlur(sampler2D image, vec2 texCoord, int kernelSize, float strength) {
    if (kernelSize > 7) { kernelSize = 7; }
    vec2 texelSize = pushConstants.texelSize * strength;
    int halfKernel = kernelSize / 2;

    // Compute Gaussian weights
    float weights[7*7]; // Maximum size
    float sum = 0.0;
    for (int i = 0; i < kernelSize; i++) {
        for (int j = 0; j < kernelSize; j++) {
            int index = i * kernelSize + j;
            float x = float(i - halfKernel) * texelSize.x;
            float y = float(j - halfKernel) * texelSize.y;
            weights[index] = exp(-(x * x + y * y) / 2.0);
            sum += weights[index];
        }
    }

    // Normalize weights
    for (int i = 0; i < kernelSize * kernelSize; i++) {
        weights[i] /= sum;
    }

    // Apply the convolution
    vec4 color = vec4(0.0);
    for (int i = -halfKernel; i <= halfKernel; i++) {
        for (int j = -halfKernel; j <= halfKernel; j++) {
            vec2 offset = vec2(float(i), float(j)) * texelSize;
            int index = (i + halfKernel) * kernelSize + (j + halfKernel);
            color += texture(image, texCoord + offset) * weights[index];
        }
    }

    return color;
}

layout(binding = BINDING_GLOBAL_BUFFER) uniform GobalUniformBufferObject {
    int kernelSize;
    float strength;
} global;
