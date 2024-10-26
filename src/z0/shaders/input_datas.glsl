#extension GL_EXT_debug_printf: enable

#define BINDING_GLOBAL_BUFFER      0
#define BINDING_MODELS_BUFFER      1
#define BINDING_MATERIALS_BUFFER   2
#define BINDING_MATERIALS_TEXTURES 3
#define BINDING_LIGHTS_BUFFER      4
#define BINDING_SHADOW_MAPS        5
#define BINDING_SHADOW_CUBEMAPS    6
#define BINDING_PBR_ENV_MAP        7
#define BINDING_PBR_IRRADIANCE_MAP 8
#define BINDING_PBR_BRDF_LUT       9

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

#define TRANSPARENCY_DISABLED      0
#define TRANSPARENCY_ALPHA         1
#define TRANSPARENCY_SCISSOR       2
#define TRANSPARENCY_SCISSOR_ALPHA 3

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
    vec4    albedoColor;
    int     transparency;
    float   alphaScissor;
    float   metallicFactor;
    float   roughnessFactor;
    vec3    emissiveFactor;
    int     diffuseIndex;
    int     specularIndex;
    int     normalIndex;
    int     metallicIndex;
    int     roughnessIndex;
    int     emissiveIndex;
    int     ambientOcclusionIndex;
    uint    metallicChannel;
    uint    roughnessChannel;
    uint    ambientOcclusionChannel;
    bool    hasTextureTransform;
    vec2    textureOffset;
    vec2    textureScale;
    vec4    parameters[4]; // ShaderMaterial::MAX_PARAMETERS
};

layout(set = 0, binding = BINDING_GLOBAL_BUFFER) uniform GlobalUniformBuffer  {
    mat4 projection;
    mat4 view;
    vec3 cameraPosition;
    uint lightsCount;
    vec4 ambient;
    bool ambientIBL;
} global;

layout(set = 0, binding = BINDING_MODELS_BUFFER) uniform ModelUniformBuffer  {
    mat4 model[1000];  // arbitrary value for debug with RenderDoc
} models;

layout(set = 0, binding = BINDING_MATERIALS_BUFFER) uniform MaterialUniformBuffer  {
    Material material[200]; // SceneRenderer::MAX_MATERIALS
} materials;

layout(set = 0, binding = BINDING_MATERIALS_TEXTURES) uniform sampler2D texSampler[200]; // SceneRenderer::MAX_IMAGES

layout(set = 0, binding = BINDING_LIGHTS_BUFFER) uniform lightArray {
    Light light[10]; // arbitrary value for debug with RenderDoc
} lights;

layout(set = 0, binding = BINDING_SHADOW_MAPS) uniform sampler2DArray shadowMaps[10]; // SceneRenderer::MAX_SHADOW_MAPS
layout(set = 0, binding = BINDING_SHADOW_CUBEMAPS) uniform samplerCube shadowMapsCubemap[10];  // SceneRenderer::MAX_SHADOW_MAPS

layout(set = 0, binding = BINDING_PBR_ENV_MAP) uniform samplerCube specularTexture;
layout(set = 0, binding = BINDING_PBR_IRRADIANCE_MAP) uniform samplerCube irradianceTexture;
layout(set = 0, binding = BINDING_PBR_BRDF_LUT) uniform sampler2D specularBRDF_LUT;

layout(push_constant) uniform PushConstants {
    int modelIndex;
    int materialIndex;
} pushConstants;
