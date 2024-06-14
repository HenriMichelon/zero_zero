#version 450
#include "tools.glsl"

layout (location = 0) out vec4 COLOR;
layout (location = 0) in vec2 UV;
layout (binding = 5) uniform sampler2D shadowMap;

void main() {
	float depth = texture(shadowMap, UV).r;
	//COLOR = vec4(vec3(LinearizeDepth(depth) / far), 1.0);
	//COLOR = texture(shadowMap, UV);
	COLOR = texture(shadowMap, UV);
}