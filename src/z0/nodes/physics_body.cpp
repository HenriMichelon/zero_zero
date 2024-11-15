/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/EActivation.h>
#include <glm/gtx/quaternion.hpp>
#include "z0/libraries.h"

module z0.PhysicsBody;

import z0.Tools;
import z0.CollisionObject;
import z0.Shape;
import z0.ConvexHullShape;

namespace z0 {


    void PhysicsBody::setGravityScale(const float value) {
        assert(!_getBodyId().IsInvalid());
        bodyInterface.SetGravityFactor(_getBodyId(), value);
    }

    PhysicsBody::PhysicsBody(const shared_ptr<Shape> &shape,
                             const uint32_t           layer,
                             const uint32_t           mask,
                             const JPH::EActivation   activationMode,
                             const JPH::EMotionType   motionType,
                             const string &           name,
                             const Type               type):
        CollisionObject{shape, layer, mask, name, type},
        motionType{motionType} {
        this->activationMode = activationMode;
        setShape(shape);
    }

    PhysicsBody::PhysicsBody(const uint32_t         layer,
                             const uint32_t         mask,
                             const JPH::EActivation activationMode,
                             const JPH::EMotionType motionType,
                             const string &         name,
                             const Type             type):
        CollisionObject{layer, mask, name, type},
        motionType{motionType} {
        this->activationMode = activationMode;
    }

    void PhysicsBody::setShape(const shared_ptr<Shape> &shape) {
        const auto &position = getPositionGlobal();
        const auto &quat     = normalize(toQuat(mat3(worldTransform)));
        this->shape = shape;
        const JPH::BodyCreationSettings settings{
                shape->_getShapeSettings(),
                JPH::RVec3{position.x, position.y, position.z},
                JPH::Quat{quat.x, quat.y, quat.z, quat.w},
                motionType,
                collisionLayer << 4 | collisionMask
        };
        setBodyId(bodyInterface.CreateAndAddBody(settings, JPH::EActivation::DontActivate));
    }

    void PhysicsBody::recreateBody() {
        setShape(dynamic_pointer_cast<Shape>(shape->duplicate()));
    }

    void PhysicsBody::setProperty(const string &property, const string &value) {
        CollisionObject::setProperty(property, value);
        if (property == "shape") {
            // split shape class name from parameters
            const auto &parts = split(value, ';');
            // we must have at least a class name
            if (parts.size() > 0) {
                if (parts.at(0) == "ConvexHullShape") {
                    if (parts.size() > 2) { die("Missing parameter for ConvexHullShape for", name); }
                    // get the children who provide the mesh for the shape
                    const auto mesh = getChild(parts[1].data());
                    if (mesh == nullptr) { die("Child with path", parts[1].data(), "not found in", name); }
                    if (mesh->getType() != MESH_INSTANCE) { die("Child with path", parts[1].data(), "not a MeshInstance in", name); }
                    setShape(make_shared<ConvexHullShape>(mesh, name));
                } else if (parts.at(0) == "BoxShape") {
                    if (parts.size() > 2) { die("Missing parameter for BoxShape for", name); }
                    setShape(make_shared<BoxShape>(to_vec3(parts[1].data()), name));
                } else {
                    die("Missing or bad shape for ", name);
                }
            }
        }
    }

}
