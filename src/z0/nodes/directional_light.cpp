#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/nodes/light.h"
#include "z0/nodes/directional_light.h"
#endif

namespace z0 {

    DirectionalLight::DirectionalLight(const string name): Light{name} {};

    DirectionalLight::DirectionalLight(vec3 lightDirection, vec4 color, float specular, const string nodeName):
        Light{color, specular, nodeName},
        direction{normalize(lightDirection)}  {}

    void DirectionalLight::setPosition(vec3 pos) {
       die("DirectionalLight can't be moved");
    }


}