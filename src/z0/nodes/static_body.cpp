#include "z0/nodes/static_body.h"

namespace z0 {

    StaticBody::StaticBody(shared_ptr<Shape> shape, uint32_t layer, uint32_t mask, const string& name):
        PhysicsBody(shape, layer, mask, JPH::EActivation::DontActivate, JPH::EMotionType::Static, name) {

    }

}