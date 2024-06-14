float near = 0.1; // cf ShadowMapFrameBuffer
float far  = 50.0; // cf ShadowMapFrameBuffer

float LinearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0; // back to NDC
    return (2.0 * near * far) / (far + near - z * (far - near));
}
