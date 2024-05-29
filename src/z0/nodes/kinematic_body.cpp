#include "z0/nodes/kinematic_body.h"

namespace z0 {

    KinematicBody::KinematicBody(shared_ptr<Shape> shape,
                         uint32_t layer,
                         uint32_t mask,
                         const string& name):
            PhysicsBody(shape,
                        layer,
                        mask,
                        JPH::EActivation::Activate,
                        JPH::EMotionType::Kinematic,
                        name) {
    }

}