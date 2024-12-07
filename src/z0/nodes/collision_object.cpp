/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/EActivation.h>
#include <Jolt/Physics/Body/Body.h>
#include <glm/gtx/quaternion.hpp>
#include "z0/libraries.h"

module z0.nodes.CollisionObject;

import z0.Application;
import z0.Constants;
import z0.Signal;
import z0.Tools;

import z0.nodes.Node;

import z0.resources.Shape;

namespace z0 {

    CollisionObject::CollisionObject(const shared_ptr<Shape> &_shape,
                                     const uint32_t           layer,
                                     const uint32_t           mask,
                                     const string &           name,
                                     const Type               type):
        Node{name, type},
        collisionLayer{layer},
        collisionMask{mask},
        shape{_shape},
        activationMode{JPH::EActivation::Activate},
        bodyInterface{Application::get()._getBodyInterface()} {
    }

    CollisionObject::CollisionObject(const uint32_t layer,
                                     const uint32_t mask,
                                     const string & name,
                                     const Type     type):
        Node{name, type},
        collisionLayer{layer},
        collisionMask{mask},
        activationMode{JPH::EActivation::Activate},
        bodyInterface{Application::get()._getBodyInterface()} {
    }

    void CollisionObject::releaseBodyId() {
        if (!bodyId.IsInvalid()) {
            if (bodyInterface.IsAdded(bodyId)) {
                bodyInterface.RemoveBody(bodyId);
            }
            bodyInterface.DestroyBody(bodyId);
            bodyId = JPH::BodyID{JPH::BodyID::cInvalidBodyID};
        }
    }

    CollisionObject::~CollisionObject() {
        releaseBodyId();
    }

    void CollisionObject::setCollisionLayer(const uint32_t layer, const bool value) {
        if (value) {
            collisionLayer |= layer;
        } else {
            collisionLayer &= ~layer;
        }
        if (!bodyId.IsInvalid()) {
            bodyInterface.SetObjectLayer(bodyId, collisionLayer << PHYSICS_LAYERS_BITS | collisionMask);
        }
    }

    void CollisionObject::setCollisionMask(const uint32_t layer, const bool value) {
        if (value) {
            collisionMask |= layer;
        } else {
            collisionMask &= ~layer;
        }
        if (!bodyId.IsInvalid()) {
            bodyInterface.SetObjectLayer(bodyId, collisionLayer << PHYSICS_LAYERS_BITS | collisionMask);
        }
    }

    void CollisionObject::setVelocity(const vec3 velocity) {
        if (bodyId.IsInvalid()) { return; }
        if (velocity == VEC3ZERO) {
            bodyInterface.SetLinearVelocity(bodyId, JPH::Vec3::sZero());
        } else {
            // current orientation * velocity
            const auto vel = toQuat(mat3(localTransform)) * velocity;
            bodyInterface.SetLinearVelocity(bodyId, JPH::Vec3{vel.x, vel.y, vel.z});
        }
    }

    vec3 CollisionObject::getVelocity() const {
        if (bodyId.IsInvalid()) { return VEC3ZERO; }
        const auto velocity = bodyInterface.GetLinearVelocity(bodyId);
        return vec3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
    }

    void CollisionObject::applyForce(const vec3 force) const {
        if (bodyId.IsInvalid()) { return; }
        bodyInterface.AddForce(
                bodyId,
                JPH::Vec3{force.x, force.y, force.z});
    }

    void CollisionObject::applyForce(const vec3 force, const vec3 position) const {
        if (bodyId.IsInvalid()) { return; }
        bodyInterface.AddForce(
                bodyId,
                JPH::Vec3{force.x, force.y, force.z},
                JPH::Vec3{position.x, position.y, position.z});
    }

    bool CollisionObject::wereInContact(const CollisionObject *obj) const {
        if (bodyId.IsInvalid()) { return false; }
        return Application::get()._getPhysicsSystem().WereBodiesInContact(bodyId, obj->bodyId);
    }

    void CollisionObject::setProperty(const string &property, const string &value) {
        Node::setProperty(property, value);
        if (property == "layer") {
            setCollisionLayer(stoul(value), true);
        } else if (property == "mask") {
            setCollisionMask(stoul(value), true);
        }
    }

    void CollisionObject::setPositionAndRotation() {
        if (updating || bodyId.IsInvalid() || !bodyInterface.IsAdded(bodyId)) {
            return;
        }
        const auto position = getPositionGlobal();
        const auto quat = normalize(toQuat(mat3(worldTransform)));
        bodyInterface.SetPositionAndRotation(
                bodyId,
                JPH::RVec3(position.x, position.y, position.z),
                JPH::Quat(quat.x, quat.y, quat.z, quat.w),
                activationMode);
    }

    void CollisionObject::setBodyId(const JPH::BodyID id) {
        bodyId = id;
        bodyInterface.SetUserData(bodyId, reinterpret_cast<uint64>(this));
        // log(toString(), " body id ", to_string(id.GetIndexAndSequenceNumber()), getName());
    }

    CollisionObject *CollisionObject::_getByBodyId(const JPH::BodyID id) {
        return reinterpret_cast<CollisionObject *>(Application::get()._getBodyInterface().GetUserData(id));
    }

    void CollisionObject::_updateTransform() {
        Node::_updateTransform();
        setPositionAndRotation();
    }

    void CollisionObject::_updateTransform(const mat4 &parentMatrix) {
        Node::_updateTransform(parentMatrix);
        setPositionAndRotation();
    }

    void CollisionObject::_physicsUpdate(const float delta) {
        Node::_physicsUpdate(delta);
        if (bodyId.IsInvalid() || !bodyInterface.IsAdded(bodyId)) { return; }
        updating = true;
        JPH::Vec3 position;
        JPH::Quat rotation;
        bodyInterface.GetPositionAndRotation(bodyId, position, rotation);
        const auto pos = vec3{position.GetX(), position.GetY(), position.GetZ()};
        if (pos != getPositionGlobal()) {
            setPositionGlobal(pos);
        }
        const auto rot = quat{rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ()};
        if (rot != getRotationQuaternion()) {
            setRotation(rot);
        }
        updating = false;
    }

    void CollisionObject::_onEnterScene() {
        if (isProcessed() && !bodyId.IsInvalid()) {
            if (!bodyInterface.IsAdded(bodyId)) {
                bodyInterface.AddBody(bodyId, activationMode);
                // log("_onEnterScene add", this->getName());
                Application::get()._setOptimizeBroadPhase();
            }
            bodyInterface.SetObjectLayer(bodyId, collisionLayer << PHYSICS_LAYERS_BITS | collisionMask);
            setPositionAndRotation();
        }
        Node::_onEnterScene();
    }

    void CollisionObject::_onExitScene() {
        if (isProcessed() && !bodyId.IsInvalid() && bodyInterface.IsAdded(bodyId)) {
            bodyInterface.RemoveBody(bodyId);
            // Application::get()._setOptimizeBroadPhase();
            // log("_onExitScene remove", this->getName());
        }
        Node::_onExitScene();
    }

    void CollisionObject::_onPause() {
        if (isProcessed()  && !bodyId.IsInvalid() && bodyInterface.IsAdded(bodyId)) {
            bodyInterface.RemoveBody(bodyId);
            // Application::get()._setOptimizeBroadPhase();
            // log("_onPause remove", this->getName());
        }
    }

    void CollisionObject::_onResume() {
        if (isProcessed() && !bodyId.IsInvalid()) {
            if (visible) {
                if (!bodyInterface.IsAdded(bodyId)) {
                    bodyInterface.AddBody(bodyId, activationMode);
                    // log("_onResume add", this->getName());
                    Application::get()._setOptimizeBroadPhase();
                }
                bodyInterface.SetObjectLayer(bodyId, collisionLayer << PHYSICS_LAYERS_BITS | collisionMask);
                setPositionAndRotation();
            }
        }
    }

    void CollisionObject::setVisible(const bool visible) {
        if (!bodyId.IsInvalid() && visible != this->visible) {
            if (visible) {
                if (!bodyInterface.IsAdded(bodyId)) {
                    // log("setVisible add", this->getName());
                    bodyInterface.AddBody(bodyId, activationMode);
                    Application::get()._setOptimizeBroadPhase();
                }
                bodyInterface.SetObjectLayer(bodyId, collisionLayer << PHYSICS_LAYERS_BITS | collisionMask);
                setPositionAndRotation();
            } else {
                if (bodyInterface.IsAdded(bodyId)) {
                    bodyInterface.RemoveBody(bodyId);
                    // Application::get()._setOptimizeBroadPhase();
                    // log("setVisible remove", this->getName());
                }
            }
        }
        Node::setVisible(visible);
    }

}
