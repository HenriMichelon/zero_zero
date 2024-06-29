#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/nodes/viewport.h"
#endif

namespace z0 {

    Viewport::Viewport(vec2 _pos, vec2 _size, const string& name):
        Node{name}, position{_pos}, size{_size} {
    }

    void Viewport::setViewportSize(vec2 _size) {
        size = _size;
    }

    void Viewport::setViewportPosition(vec2 _position) {
        position = _position;
    }

}