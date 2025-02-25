/*
 * Copyright (c) 2024-2025 Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyLock.h>
#include <Jolt/Physics/EActivation.h>
#include <glm/gtx/quaternion.hpp>
#include "z0/libraries.h"

module z0.nodes.PhysicsBody;

import z0.Application;
import z0.Constants;
import z0.Tools;

import z0.nodes.CollisionObject;
import z0.nodes.MeshInstance;

import z0.resources.ConvexHullShape;
import z0.resources.MeshShape;
import z0.resources.Shape;
import z0.resources.StaticCompoundShape;
import z0.resources.SubShape;

namespace z0 {

    PhysicsBody::PhysicsBody(const shared_ptr<Shape> &shape,
                             const uint32_t           layer,
                             const JPH::EActivation   activationMode,
                             const JPH::EMotionType   motionType,
                             const string &           name,
                             const Type               type):
        CollisionObject{shape, layer, name, type},
        motionType{motionType} {
        this->activationMode = activationMode;
        setShape(shape);
    }

    PhysicsBody::PhysicsBody(const uint32_t         layer,
                             const JPH::EActivation activationMode,
                             const JPH::EMotionType motionType,
                             const string &         name,
                             const Type             type):
        CollisionObject{layer, name, type},
        motionType{motionType} {
        this->activationMode = activationMode;
    }

    void PhysicsBody::setShape(const shared_ptr<Shape> &shape) {
        releaseBodyId();
        const auto &position = getPositionGlobal();
        const auto &quat     = normalize(getRotationQuaternion());
        this->shape = shape;
        const JPH::BodyCreationSettings settings{
                shape->_getShapeSettings(),
                JPH::RVec3{position.x, position.y, position.z},
                JPH::Quat{quat.x, quat.y, quat.z, quat.w},
                motionType,
                collisionLayer,
        };
        const auto body = bodyInterface.CreateBody(settings);
        setBodyId(body->GetID());
    }

    void PhysicsBody::recreateBody() {
        setShape(dynamic_pointer_cast<Shape>(shape->duplicate()));
    }


    void PhysicsBody::setVelocity(const vec3& velocity) {
        if (bodyId.IsInvalid()) { return; }
        if (velocity == VEC3ZERO) {
            bodyInterface.SetLinearVelocity(bodyId, JPH::Vec3::sZero());
        } else {
            // current orientation * velocity
            const auto vel = getRotationQuaternion() * velocity;
            bodyInterface.SetLinearVelocity(bodyId, JPH::Vec3{vel.x, vel.y, vel.z});
        }
    }

    void PhysicsBody::setGravityFactor(const float factor) {
        if (bodyId.IsInvalid()) { return; }
        bodyInterface.SetGravityFactor(bodyId, factor);
    }

    vec3 PhysicsBody::getVelocity() const {
        if (bodyId.IsInvalid()) { return VEC3ZERO; }
        const auto velocity = bodyInterface.GetLinearVelocity(bodyId);
        return vec3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
    }

    void PhysicsBody::applyForce(const vec3& force) const {
        if (bodyId.IsInvalid()) { return; }
        bodyInterface.AddForce(
                bodyId,
                JPH::Vec3{force.x, force.y, force.z});
    }

    void PhysicsBody::applyForce(const vec3& force, const vec3& position) const {
        if (bodyId.IsInvalid()) { return; }
        bodyInterface.AddForce(
                bodyId,
                JPH::Vec3{force.x, force.y, force.z},
                JPH::Vec3{position.x, position.y, position.z});
    }

    void PhysicsBody::setMass(const float value) const {
        assert(!_getBodyId().IsInvalid());
        const JPH::BodyLockWrite lock(app()._getPhysicsSystem().GetBodyLockInterface(), _getBodyId());
        if (lock.Succeeded()) {
            JPH::MotionProperties *mp = lock.GetBody().GetMotionProperties();
            if (value != 0.0f) {
                mp->SetInverseMass(1.0f/value);
            } else {
                mp->SetInverseMass(0.0f);
            }
        }
    }

    void PhysicsBody::setBounce(const float value) const {
        assert(!_getBodyId().IsInvalid());
        bodyInterface.SetRestitution(_getBodyId(), value);
    }

    void PhysicsBody::setProperty(const string &property, const string &value) {
        CollisionObject::setProperty(property, value);
        if (property == "shape") {
            // split shape class name from parameters
            const auto parts = split(value, ';');
            // we must have at least a class name
            if (parts.size() > 0) {
                if (parts.at(0) == "ConvexHullShape") {
                    if (parts.size() > 2) { die("Missing parameter for ConvexHullShape for", getName()); }
                    // get the children who provide the mesh for the shape
                    const auto mesh = getChild(parts[1].data());
                    if (mesh == nullptr) { die("Child with path", parts[1].data(), "not found in", getName()); }
                    if (mesh->getType() != MESH_INSTANCE) { die("Child with path", parts[1].data(), "not a MeshInstance in", getName()); }
                    setShape(make_shared<ConvexHullShape>(mesh, getName()));
                } else if (parts.at(0) == "BoxShape") {
                    if (parts.size() < 2) { die("Missing parameter for BoxShape for", getName()); }
                    setShape(make_shared<BoxShape>(to_vec3(parts[1].data()), getName()));
                } else if (parts.at(0) == "SphereShape") {
                    if (parts.size() < 2) { die("Missing parameter for SphereShape for", getName()); }
                    setShape(make_shared<SphereShape>(stof(parts[1].data()), getName()));
                } else if (parts.at(0) == "CylinderShape") {
                    if (parts.size() < 3) { die("Missing parameter for CylinderShape for", getName()); }
                    setShape(make_shared<CylinderShape>(stof(parts[1].data()), stof(parts[2].data()), getName()));
                } else if (parts.at(0) == "MeshShape") {
                    setShape(make_shared<MeshShape>(*this));
                } else if (parts.at(0) == "AABBShape") {
                    setShape(make_shared<AABBShape>(*this));
                } else if (parts.at(0) == "StaticCompoundShape") {
                    vector<SubShape> subShapes;
                    for (const auto &meshInstance : findAllChildren<MeshInstance>()) {
                        subShapes.push_back({
                            make_shared<MeshShape>(meshInstance),
                            meshInstance->getPositionGlobal(),
                            meshInstance->getRotationGlobal()
                        });
                    }
                    setShape(make_shared<StaticCompoundShape>(subShapes));
                } else {
                    die("PhysicsBody : missing or invalid shape for ", getName());
                }
            }
        } else if (property == "mass") {
            setMass(stof(value));
        } else if (property == "bounce") {
            setBounce(stof(value));
        }
    }

}
