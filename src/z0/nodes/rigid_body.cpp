#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/resources/shape.h"
#include "z0/nodes/physics_node.h"
#include "z0/nodes/physics_body.h"
#include "z0/nodes/rigid_body.h"
#endif

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