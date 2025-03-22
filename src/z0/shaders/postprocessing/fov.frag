/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 450

#include "blur.glsl"

void main() {
    COLOR = gaussianBlur(inputImage, UV,  global.kernelSize,  exp(global.strength * texture(depthBuffer, UV).r) - 0.5);
}

