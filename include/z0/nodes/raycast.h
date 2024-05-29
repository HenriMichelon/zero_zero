#pragma once

#include "z0/nodes/node.h"

namespace z0 {

    class RayCast : public Node {
    public:
        RayCast(const string& name);

    private:
        vec3 target{0.0};

    public:
        void _physicsUpdate() override;
    };

}
