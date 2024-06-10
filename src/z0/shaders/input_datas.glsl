layout(set = 0, binding = 0) uniform GlobalUniformBuffer  {
    mat4 projection;
    mat4 view;
    vec4 ambient;
    vec3 cameraPosition;
} global;

layout(set = 0, binding = 1) uniform ModelUniformBuffer  {
    mat4 matrix;
} model;

layout(set = 0, binding = 2) uniform MaterialUniformBuffer  {
    int transparency;
    float alphaScissor;
    int diffuseIndex;
    int specularIndex;
    int normalIndex;
    vec4 albedoColor;
    float shininess;
    vec4 parameters[4]; // ShaderMaterial::MAX_PARAMETERS
} material;

layout(set = 0, binding = 3) uniform sampler2D texSampler[200]; // SceneRenderer::MAX_IMAGES

struct VertexOut {
    vec2 UV;
    vec3 NORMAL;
    vec4 GLOBAL_POSITION;
    vec3 POSITION;
    vec3 VIEW_DIRECTION;
    mat3 TBN;
    vec4 tangent;
};