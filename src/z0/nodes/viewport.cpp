module;
#include "z0/jolt.h"
#include "z0/libraries.h"

module z0;

import :Viewport;

namespace z0 {

    Viewport::Viewport(const vec2 &position, const vec2 &size, const string &name):
        Node{name, VIEWPORT}, position{position}, size{size} {
    }

    Viewport::Viewport(const string &name):
        Node{name, VIEWPORT} {
    }

}
