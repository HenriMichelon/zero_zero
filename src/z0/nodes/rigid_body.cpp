#include "z0/nodes/rigid_body.h"

namespace z0 {

    RigidBody::RigidBody(shared_ptr<Shape> shape,
                         uint32_t layer,
                         uint32_t mask,
                         const string& name):
        PhysicsBody(shape,
                    layer,
                    mask,
                    JPH::EActivation::Activate,
                    JPH::EMotionType::Dynamic, name) {
    }

    void RigidBody::setBounce(float value) {
        bodyInterface.SetRestitution(_getBodyId(), value);
    }


}