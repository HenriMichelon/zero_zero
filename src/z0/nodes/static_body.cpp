module;
#include "z0/jolt.h"
#include "z0/libraries.h"

module z0;

import :PhysicsBody;
import :Shape;

namespace z0 {

    StaticBody::StaticBody(const shared_ptr<Shape> &shape,
                           const uint32_t           layer,
                           const string &           name):
        PhysicsBody(shape,
                    layer,
                    0,
                    JPH::EActivation::DontActivate,
                    JPH::EMotionType::Static,
                    name,
                    STATIC_BODY) {
    }

    StaticBody::StaticBody(const uint32_t layer,
                           const string & name):
        PhysicsBody(layer,
                    0,
                    JPH::EActivation::DontActivate,
                    JPH::EMotionType::Static,
                    name,
                    STATIC_BODY) {
    }

    StaticBody::StaticBody(const string &name):
        PhysicsBody(0,
                    0,
                    JPH::EActivation::DontActivate,
                    JPH::EMotionType::Static,
                    name,
                    STATIC_BODY) {
    }

}
