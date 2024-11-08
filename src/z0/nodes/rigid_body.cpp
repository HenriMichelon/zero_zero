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
#include <cassert>

#include "z0/libraries.h"

module z0.RigidBody;

import z0.Shape;

namespace z0 {

    RigidBody::RigidBody(const shared_ptr<Shape> &shape,
                         const uint32_t           layer,
                         const uint32_t           mask,
                         const string &           name):
        PhysicsBody(shape,
                    layer,
                    mask,
                    JPH::EActivation::Activate,
                    JPH::EMotionType::Dynamic,
                    name,
                    RIGID_BODY) {
    }


    RigidBody::RigidBody(const string &name):
        PhysicsBody(0,
                    0,
                    JPH::EActivation::Activate,
                    JPH::EMotionType::Dynamic,
                    name,
                    RIGID_BODY) {
    }

    void RigidBody::setBounce(const float value) {
        assert(!_getBodyId().IsInvalid());
        bodyInterface.SetRestitution(_getBodyId(), value);
    }

    void RigidBody::setProperty(const string &property, const string &value) {
        PhysicsBody::setProperty(property, value);
        if (property == "bounce") { setBounce(stof(value)); }
    }
}
