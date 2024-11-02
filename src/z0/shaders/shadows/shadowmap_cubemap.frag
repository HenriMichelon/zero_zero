/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#version 460
#include "shadowmap_input.glsl"

layout (location = 0) in vec4 WORLD_POSITION;

layout (location = 0) out vec4 COLOR;

void main() {
    // TODO transparency
    gl_FragDepth = length(WORLD_POSITION.xyz - global.lightPosition) / global.farPlane;
}
