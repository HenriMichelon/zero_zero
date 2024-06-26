#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/nodes/environment.h"
#endif

namespace z0 {

    void Environment::setProperty(const string&property, const string& value) {
        Node::setProperty(property, value);
         if (property == "ambient_color") {
            setAmbientColorAndIntensity(to_vec4(value));
        }
    }

}