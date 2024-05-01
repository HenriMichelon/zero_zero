layout(set = 0, binding = 0) uniform GlobalUniformBuffer  {
    mat4 projection;
    mat4 view;
    vec4 ambient;
    vec3 cameraPosition;
} global;

layout(set = 0, binding = 1) uniform ModelUniformBuffer  {
    mat4 matrix;
} model;

struct VertexOut {
    vec2 UV;
    vec3 NORMAL;
    vec4 GLOBAL_POSITION;
    vec3 POSITION;
    vec3 VIEW_DIRECTION;
    mat3 TBN;
    vec4 tangent;
};