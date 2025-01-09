/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 460
#include "shadowmap_input.glsl"

layout (location = 0) in vec3 position;

layout (location = 0) out vec4 WORLD_POSITION;

void main() {
    if (pushConstants.transparency != 0) {
        // discard in case of transparency, any mode
        gl_Position = vec4(0);
    } else {
        WORLD_POSITION = pushConstants.model * vec4(position, 1.0f);
        gl_Position = global.lightSpace[pushConstants.lightSpaceIndex] * WORLD_POSITION;
    }
}
