#pragma once

#include "z0/nodes/physics_body.h"

namespace z0 {

    class KinematicBody: public PhysicsBody {
    public:
        explicit KinematicBody(shared_ptr<Shape> shape,
                           uint32_t layer=1,
                           uint32_t mask=1,
                           const string& name = "KinematicBody");
        ~KinematicBody() override = default;

    };

}