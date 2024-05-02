#pragma once

#include "z0/nodes/physics_body.h"

namespace z0 {

    class StaticBody: public PhysicsBody {
    public:
        explicit StaticBody(shared_ptr<Shape> shape,
                            uint32_t layer=1,
                            uint32_t mask=1,
                            const string& name = "StaticBody");
        ~StaticBody() override = default;
    };

}