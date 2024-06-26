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

    void DirectionalLight::setProperty(const string&property, const string& value) {
        Node::setProperty(property, value);
        if (property == "direction") {
            setDirection(normalize(to_vec3(value)));
        } else if (property == "color") {
            setColorAndIntensity(to_vec4(value));
        } else if (property == "specular") {
            setSpecularIntensity(stof(value));
        } else if (property == "cast_shadow") {
            setCastShadow(value == "true");
        }
    }

}