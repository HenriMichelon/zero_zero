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

module z0.KinematicBody;

import z0.Node;
import z0.Shape;

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

    shared_ptr<Node> KinematicBody::duplicateInstance() {
        return make_shared<KinematicBody>(*this);
    }

}
