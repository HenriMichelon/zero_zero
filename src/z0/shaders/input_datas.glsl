#extension GL_EXT_debug_printf: enable

struct VertexOut {
    vec2    UV;
    vec3    NORMAL;
    vec4    GLOBAL_POSITION;
    vec3    POSITION;
    vec3    VIEW_DIRECTION;
    float   CLIPSPACE_Z;
    mat3    TBN;
    vec4    TANGENT;
};

// Light::LightType
#define LIGHT_DIRECTIONAL 0
#define LIGHT_OMNI        1
#define LIGHT_SPOT        2

struct Light {
    // light params
    int     type; // Light::LightType
    vec3    position;
    vec3    direction;
    vec4    color;
    float   specular;
    float   range;
    float   cutOff;
    float   outerCutOff;
    // shadow map params
    int     mapIndex;
    float   farPlane;
    uint    cascadesCount;
    vec4    cascadeSplitDepth;
    mat4    lightSpace[6];
};

struct Material  {
    int     transparency;
    float   alphaScissor;
    int     diffuseIndex;
    int     specularIndex;
    int     normalIndex;
    vec4    albedoColor;
    float   shininess;
    bool    hasTextureTransform;
    vec2    textureOffset;
    vec2    textureScale;
    vec4    parameters[4]; // ShaderMaterial::MAX_PARAMETERS
};

layout(set = 0, binding = 0) uniform GlobalUniformBuffer  {
    mat4 projection;
    mat4 view;
    vec4 ambient;
    vec3 cameraPosition;
    uint lightsCount;
} global;

layout(set = 0, binding = 1) uniform ModelUniformBuffer  {
    mat4 model[1000];  // arbitrary value for debug with RenderDoc
} models;

layout(set = 0, binding = 2) uniform MaterialUniformBuffer  {
    Material material[200]; // SceneRenderer::MAX_MATERIALS
} materials;

layout(set = 0, binding = 3) uniform sampler2D texSampler[200]; // SceneRenderer::MAX_IMAGES

layout(set = 0, binding = 4) uniform lightArray {
    Light light[10]; // arbitrary value for debug with RenderDoc
} lights;

layout(set = 0, binding = 5) uniform sampler2DArray shadowMaps[10]; // SceneRenderer::MAX_SHADOW_MAPS

layout(set = 0, binding = 6) uniform samplerCube shadowMapsCubemap[10];  // SceneRenderer::MAX_SHADOW_MAPS

layout(push_constant) uniform PushConstants {
    int modelIndex;
    int materialIndex;
} pushConstants;
