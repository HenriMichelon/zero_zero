
#define BINDING_GLOBAL_BUFFER     0
#define BINDING_INPUT_COLOR       1
#define BINDING_DEPTH_BUFFER      2
#define BINDING_NORMAL_COLOR      3
#define BINDING_DIFFUSE_COLOR     4

layout(binding = BINDING_INPUT_COLOR) uniform sampler2D inputImage;
layout(binding = BINDING_DEPTH_BUFFER) uniform sampler2D depthBuffer;
layout(binding = BINDING_NORMAL_COLOR) uniform sampler2D normalColor;
layout(binding = BINDING_DIFFUSE_COLOR) uniform sampler2D diffuseColor;

layout (location = 0) in vec2 UV;
layout (location = 0) out vec4 COLOR;

layout(push_constant) uniform PushConstants {
    vec2 texelSize;
} pushConstants;
