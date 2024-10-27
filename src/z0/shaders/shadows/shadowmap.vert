#version 460
#include "shadowmap_input.glsl"

layout (location = 0) in vec3 position;

void main() {
    if (pushConstants.transparency != 0) {
        // discard in case of transparency, any mode
        gl_Position = vec4(0);
    } else {
        gl_Position = global.lightSpace[pushConstants.lightSpaceIndex] * pushConstants.model * vec4(position, 1.0);
    }
}