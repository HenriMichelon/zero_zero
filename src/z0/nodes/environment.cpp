/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

module z0;

import :Environment;
import :Tools;

namespace z0 {

    Environment::Environment(
            const vec4    colorAndIntensity,
            const string &nodeName):
        Node{nodeName, ENVIRONMENT},
        ambientColorIntensity{colorAndIntensity} {
    }

    void Environment::setProperty(const string &property, const string &value) {
        Node::setProperty(property, value);
        if (property == "ambient_color") {
            setAmbientColorAndIntensity(to_vec4(value));
        }
    }

}
