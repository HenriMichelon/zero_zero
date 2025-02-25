/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

module z0.nodes.Viewport;

namespace z0 {

    Viewport::Viewport(const vec2 &position, const vec2 &size, const string &name):
        Node{name, VIEWPORT}, position{position}, size{size} {
    }

    Viewport::Viewport(const string &name):
        Node{name, VIEWPORT} {
    }

    shared_ptr<Node> Viewport::duplicateInstance() const {
        return make_shared<Viewport>(*this);
    }

}
