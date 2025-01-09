/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/

layout (binding = 0) uniform GlobalBuffer {
    vec3  lightPosition;
    float farPlane;
    mat4  lightSpace[6];
} global;

layout(push_constant) uniform PushConstants {
    mat4  model;
    uint  lightSpaceIndex;
    uint  transparency;
} pushConstants;
