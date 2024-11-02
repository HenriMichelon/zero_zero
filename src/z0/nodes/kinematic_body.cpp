/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/jolt.h"
#include "z0/libraries.h"

module z0;

import :KinematicBody;

namespace z0 {

    KinematicBody::KinematicBody(const shared_ptr<Shape> &shape,
                                 const uint32_t           layer,
                                 const uint32_t           mask,
                                 const string &           name):
        PhysicsBody(shape,
                    layer,
                    mask,
                    JPH::EActivation::Activate,
                    JPH::EMotionType::Kinematic,
                    name,
                    KINEMATIC_BODY) {
    }

    KinematicBody::KinematicBody(const string &name):
        PhysicsBody(0xff,
                    0xff,
                    JPH::EActivation::Activate,
                    JPH::EMotionType::Kinematic,
                    name,
                    KINEMATIC_BODY) {
    }

}
