#extension GL_EXT_debug_printf: enable

struct VertexOut {
    vec2 UV;
    vec3 NORMAL;
    vec4 GLOBAL_POSITION;
    vec3 POSITION;
    vec3 VIEW_DIRECTION;
    float CLIPSPACE_Z;
    mat3 TBN;
    vec4 tangent;
};

struct DirectionalLight {
    vec3 direction;
    vec4 color;
    float specular;
};

struct PointLight {
    vec3 position;
    vec4 color;
    float specular;
    float constant;
    float linear;
    float quadratic;
    bool isSpot;
    vec3 direction;
    float cutOff;
    float outerCutOff;
};

struct ShadowMap {
    mat4 lightSpace[8]; // fixed at 8 for alignements
    vec4 cascadeSplitDepth; // fixed at 4 for alignements
    bool isCubemap;
    vec3 lightPosition;
};

struct Material  {
    int transparency;
    float alphaScissor;
    int diffuseIndex;
    int specularIndex;
    int normalIndex;
    vec4 albedoColor;
    float shininess;
    bool hasTextureTransform;
    vec2 textureOffset;
    vec2 textureScale;
    vec4 parameters[4]; // ShaderMaterial::MAX_PARAMETERS
};

layout(set = 0, binding = 0) uniform GlobalUniformBuffer  {
    mat4 projection;
    mat4 view;
    vec4 ambient;
    vec3 cameraPosition;
    DirectionalLight directionalLight;
    bool haveDirectionalLight;
    int pointLightsCount;
    int cascadedShadowMapIndex;
    int cascadesCount;
    int shadowMapsCount;
} global;

layout(set = 0, binding = 1) uniform ModelUniformBuffer  {
    mat4 model[1000];  // arbitrary value for debug with RenderDoc
} models;

layout(set = 0, binding = 2) uniform MaterialUniformBuffer  {
    Material material[200]; // SceneRenderer::MAX_MATERIALS
} materials;

layout(set = 0, binding = 3) uniform sampler2D texSampler[200]; // SceneRenderer::MAX_IMAGES

layout(set = 0, binding = 4) uniform ShadowMapArray {
    ShadowMap shadowMaps[10]; // SceneRenderer::MAX_SHADOW_MAPS
} shadowMapsInfos;

layout(set = 0, binding = 5) uniform sampler2DArray shadowMaps[10]; // SceneRenderer::MAX_SHADOW_MAPS

layout(set = 0, binding = 7) uniform samplerCube shadowMapsCubemap[10];  // SceneRenderer::MAX_SHADOW_MAPS

layout(set = 0, binding = 6) uniform PointLightArray {
    PointLight lights[10]; // arbitrary value for debug with RenderDoc
} pointLights;

layout(push_constant) uniform PushConstants {
    int modelIndex;
    int materialIndex;
} pushConstants;
