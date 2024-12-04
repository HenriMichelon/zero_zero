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

module z0.CollisionObject;

import z0.Application;
import z0.Constants;
import z0.Node;
import z0.Signal;
import z0.Shape;
import z0.Tools;

namespace z0 {

    void CollisionObject::releaseBodyId() {
        if (!bodyId.IsInvalid()) {
            if (bodyInterface.IsAdded(bodyId)) {
                bodyInterface.RemoveBody(bodyId);
            }
            bodyId = JPH::BodyID{JPH::BodyID::cInvalidBodyID};
        }
    }

    CollisionObject::~CollisionObject() {
        releaseBodyId();
    }

    void CollisionObject::setCollistionLayer(const uint32_t layer, const bool value) {
        // assert(!bodyId.IsInvalid());
        if (value) {
            collisionLayer |= layer;
        } else {
            collisionLayer &= ~layer;
        }
        if (bodyInterface.IsAdded(bodyId)) {
            bodyInterface.SetObjectLayer(bodyId, collisionLayer << 4 | collisionMask);
        }
    }

    void CollisionObject::setCollistionMask(const uint32_t layer, const bool value) {
        // assert(!bodyId.IsInvalid());
        if (value) {
            collisionMask |= layer;
        } else {
            collisionMask &= ~layer;
        }
        if (bodyInterface.IsAdded(bodyId)) {
            bodyInterface.SetObjectLayer(bodyId, collisionLayer << 4 | collisionMask);
        }
    }

    void CollisionObject::setVelocity(const vec3 velocity) {
        assert(!bodyId.IsInvalid());
        if (velocity == VEC3ZERO) {
            bodyInterface.SetLinearVelocity(bodyId, JPH::Vec3::sZero());
        } else {
            // current orientation * velocity
            const auto vel = toQuat(mat3(localTransform)) * velocity;
            bodyInterface.SetLinearVelocity(bodyId, JPH::Vec3{vel.x, vel.y, vel.z});
        }
    }

    vec3 CollisionObject::getVelocity() const {
        assert(!bodyId.IsInvalid());
        const auto velocity = bodyInterface.GetLinearVelocity(bodyId);
        return vec3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
    }

    void CollisionObject::applyForce(const vec3 force) const {
        assert(!bodyId.IsInvalid());
        bodyInterface.AddForce(
                bodyId,
                JPH::Vec3{force.x, force.y, force.z});
    }

    void CollisionObject::applyForce(const vec3 force, const vec3 position) const {
        assert(!bodyId.IsInvalid());
        bodyInterface.AddForce(
                bodyId,
                JPH::Vec3{force.x, force.y, force.z},
                JPH::Vec3{position.x, position.y, position.z});
    }

    bool CollisionObject::wereInContact(const CollisionObject *obj) const {
        assert(!bodyId.IsInvalid());
        return Application::get()._getPhysicsSystem().WereBodiesInContact(bodyId, obj->bodyId);
    }

    void CollisionObject::setProperty(const string &property, const string &value) {
        Node::setProperty(property, value);
        if (property == "layer") {
            setCollistionLayer(stoul(value), true);
        } else if (property == "mask") {
            setCollistionMask(stoul(value), true);
        }
    }

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
        shape{nullptr},
        activationMode{JPH::EActivation::Activate},
        bodyInterface{Application::get()._getBodyInterface()} {
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
        // log(toString(), " body id ", to_string(id.GetIndexAndSequenceNumber()));
    }

    CollisionObject *CollisionObject::_getByBodyId(const JPH::BodyID id) const {
        assert(!bodyId.IsInvalid());
        return reinterpret_cast<CollisionObject *>(bodyInterface.GetUserData(id));
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
        if (!shape || bodyId.IsInvalid() || !bodyInterface.IsAdded(bodyId)) { return; }
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
        if (isProcessed()) {
            if (!bodyInterface.IsAdded(bodyId)) {
                bodyInterface.AddBody(bodyId, activationMode);
                bodyInterface.SetObjectLayer(bodyId, collisionLayer << 4 | collisionMask);
                setPositionAndRotation();
            }
            bodyInterface.ActivateBody(bodyId);
        }
        Node::_onEnterScene();
    }

    void CollisionObject::_onExitScene() {
        if (isProcessed() && bodyInterface.IsAdded(bodyId)) {
            bodyInterface.DeactivateBody(bodyId);
            bodyInterface.RemoveBody(bodyId);
        }
        Node::_onExitScene();
    }

    void CollisionObject::_onPause() {
        if (isProcessed() && bodyInterface.IsAdded(bodyId)) {
            bodyInterface.DeactivateBody(bodyId);
            bodyInterface.RemoveBody(bodyId);
        }
    }

    void CollisionObject::_onResume() {
        if (isProcessed()) {
            if (visible) {
                if (!bodyInterface.IsAdded(bodyId)) {
                    bodyInterface.AddBody(bodyId, activationMode);
                    bodyInterface.SetObjectLayer(bodyId, collisionLayer << 4 | collisionMask);
                    setPositionAndRotation();
                }
                bodyInterface.ActivateBody(bodyId);
            }
        }
    }

    void CollisionObject::setVisible(const bool visible) {
        Node::setVisible(visible);
        if (!bodyId.IsInvalid()) {
            if (visible) {
                if (!bodyInterface.IsAdded(bodyId)) {
                    bodyInterface.AddBody(bodyId, activationMode);
                    bodyInterface.SetObjectLayer(bodyId, collisionLayer << 4 | collisionMask);
                    setPositionAndRotation();
                }
                bodyInterface.ActivateBody(bodyId);
            } else {
                if (bodyInterface.IsAdded(bodyId)) {
                    bodyInterface.DeactivateBody(bodyId);
                    bodyInterface.RemoveBody(bodyId);
                }
            }
        }
    }

}
