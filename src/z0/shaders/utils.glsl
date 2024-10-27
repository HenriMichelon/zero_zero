#include "input_datas.glsl"

float near = 0.1; // cf ShadowMapFrameBuffer
float far  = 50.0; // cf ShadowMapFrameBuffer

float LinearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0; // back to NDC
    return (2.0 * near * far) / (far + near - z * (far - near));
}

// Apply texture UV transforms
vec2 uvTransform(const TextureInfo texture, const vec2 UV) {
    const mat3 translation = mat3(1,0,0, 0,1,0, texture.offset.x, texture.offset.y, 1);
    const mat3 scale = mat3(texture.scale.x,0,0, 0,texture.scale.y,0, 0,0,1);
    return (translation * scale * vec3(UV, 1)).xy;
}

// Converts a color from sRGB gamma to linear light gamma
vec4 toLinear(const vec4 sRGB) {
    const bvec3 cutoff = lessThan(sRGB.rgb, vec3(0.04045));
    const vec3 higher = pow((sRGB.rgb + vec3(0.055))/vec3(1.055), vec3(2.4));
    const vec3 lower = sRGB.rgb/vec3(12.92);
    return vec4(mix(higher, lower, cutoff), sRGB.a);
}