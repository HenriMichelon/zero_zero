/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/EActivation.h>
#include "z0/libraries.h"

module z0.StaticBody;

import z0.Node;
import z0.PhysicsBody;
import z0.Shape;

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

    shared_ptr<Node> StaticBody::duplicateInstance() {
        auto dup = make_shared<StaticBody>(*this);
        dup->recreateBody();
        return dup;
    }


}
