/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyLock.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/EActivation.h>
#include <cassert>

#include "z0/libraries.h"

module z0.nodes.RigidBody;

import z0.Application;

import z0.nodes.Node;

import z0.resources.Shape;

namespace z0 {

    RigidBody::RigidBody(const shared_ptr<Shape> &shape,
                         const uint32_t           layer,
                         const string &           name):
        PhysicsBody(shape,
                    layer,
                    JPH::EActivation::Activate,
                    JPH::EMotionType::Dynamic,
                    name,
                    RIGID_BODY) {
    }


    RigidBody::RigidBody(const string &name):
        PhysicsBody(0,
                    JPH::EActivation::Activate,
                    JPH::EMotionType::Dynamic,
                    name,
                    RIGID_BODY) {
    }

    void RigidBody::setMass(const float value) const {
        assert(!_getBodyId().IsInvalid());
        const JPH::BodyLockWrite lock(app()._getPhysicsSystem().GetBodyLockInterface(), _getBodyId());
        if (lock.Succeeded()) {
            JPH::MotionProperties *mp = lock.GetBody().GetMotionProperties();
            mp->SetInverseMass(1.0f/std::max<float>(0.01f, value));
        }
    }

    void RigidBody::setBounce(const float value) const {
        assert(!_getBodyId().IsInvalid());
        bodyInterface.SetRestitution(_getBodyId(), value);
    }

    void RigidBody::setProperty(const string &property, const string &value) {
        PhysicsBody::setProperty(property, value);
        if (property == "bounce") {
            setBounce(stof(value));
        } else if (property == "mass") {
            setMass(stof(value));
        }
    }

    shared_ptr<Node> RigidBody::duplicateInstance() {
        return make_shared<RigidBody>(*this);
    }

}
