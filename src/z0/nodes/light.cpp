#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/nodes/light.h"
#endif

namespace z0 {

    Light::Light(const string nodeName): Node{nodeName} {}

    Light::Light(vec4 color, float specular, const string nodeName):
            Node{nodeName}, 
            colorAndIntensity{color}, 
            specularIntensity{specular}  {}

}