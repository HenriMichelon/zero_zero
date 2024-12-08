/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Character/Character.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <glm/gtx/quaternion.hpp>
#include "z0/libraries.h"

module z0.nodes.Character;

import z0.Application;
import z0.Constants;
import z0.Tools;

import z0.nodes.CollisionObject;

import z0.resources.Shape;

namespace z0 {
    Character::Character(const float    height,
                         const float    radius,
                         const uint32_t layer,
                         const uint32_t mask,
                         const string & name):
        CollisionObject(layer,
                        mask,
                        name,
                        CHARACTER) {
        setShape(height, radius);
    }

    void Character::setShape(const float height, const float radius) {
        assert(height/2 > radius);
        if (physicsCharacter) {
            physicsCharacter->RemoveFromPhysicsSystem();
        }
        yDelta = height/2;
        const auto position= getPositionGlobal();
        const auto quat = normalize(getRotationQuaternion());
        const auto pos = JPH::RVec3(position.x, position.y + yDelta, position.z);
        const auto rot = JPH::Quat(quat.x, quat.y, quat.z, quat.w);

        JPH::CharacterVirtualSettings settingsVirtual;
        settingsVirtual.mShape         = new JPH::CapsuleShape(height/2 - radius, radius);
        settingsVirtual.mMaxSlopeAngle = radians(30.0);
        settingsVirtual.mEnhancedInternalEdgeRemoval = true;
        settingsVirtual.mUp = JPH::Vec3{upVector.x, upVector.y, upVector.z};
        virtualCharacter               = make_unique<JPH::CharacterVirtual>(&settingsVirtual,
                                                       pos,
                                                       rot,
                                                       reinterpret_cast<uint64>(this),
                                                       &Application::get()._getPhysicsSystem());
        virtualCharacter->SetListener(this);

        JPH::CharacterSettings settings;
        settings.mLayer  = collisionLayer << PHYSICS_LAYERS_BITS | collisionMask;
        settings.mShape  = settingsVirtual.mShape;
        settings.mMaxSlopeAngle = settingsVirtual.mMaxSlopeAngle;
        settings.mEnhancedInternalEdgeRemoval = settingsVirtual.mEnhancedInternalEdgeRemoval;
        settings.mUp = settingsVirtual.mUp;
        physicsCharacter = make_unique<JPH::Character>(&settings,
                                                       pos,
                                                       rot,
                                                       reinterpret_cast<uint64>(this),
                                                       &Application::get()._getPhysicsSystem());
        physicsCharacter->AddToPhysicsSystem();
        setBodyId(physicsCharacter->GetBodyID());
    }

    Character::~Character() {
        if (physicsCharacter && bodyInterface.IsAdded(bodyId)) {
            physicsCharacter->RemoveFromPhysicsSystem();
        }
        bodyId = JPH::BodyID{JPH::BodyID::cInvalidBodyID};
    }

    vec3 Character::getGroundVelocity() const {
        const auto velocity = physicsCharacter->GetGroundVelocity();
        return vec3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
    }

    void Character::setUpVector(const vec3 vector) {
        upVector = vector;
        virtualCharacter->SetUp(JPH::Vec3{upVector.x, upVector.y, upVector.z});
        physicsCharacter->SetUp(JPH::Vec3{upVector.x, upVector.y, upVector.z});
    }

    list<CollisionObject::Collision> Character::getCollisions() const {
        list<Collision> contacts;
        for (const auto &contact : virtualCharacter->GetActiveContacts()) {
            auto *node = reinterpret_cast<CollisionObject *>(bodyInterface.GetUserData(contact.mBodyB));
            assert(node && "physics body not associated with a node");
            contacts.push_back({
                .position = vec3{contact.mPosition.GetX(), contact.mPosition.GetY(), contact.mPosition.GetZ()},
                .normal = vec3{contact.mSurfaceNormal.GetX(),
                               contact.mSurfaceNormal.GetY(),
                               contact.mSurfaceNormal.GetZ()},
                .object = node
            });
        }
        return contacts;
    }

    void Character::setVelocity(const vec3 velocity) {
        if (!bodyInterface.IsAdded(bodyId)) { return; }
        if (velocity == VEC3ZERO) {
            physicsCharacter->SetLinearVelocity(JPH::Vec3::sZero());
        } else {
            // current orientation * velocity
            const auto vel = getRotationQuaternion() * velocity;
            physicsCharacter->SetLinearVelocity(JPH::Vec3{vel.x, vel.y, vel.z});
        }
        virtualCharacter->SetLinearVelocity(physicsCharacter->GetLinearVelocity());
    }

    void Character::setPositionAndRotation() {
        if (updating || bodyId.IsInvalid() || !bodyInterface.IsAdded(bodyId)) {
            return;
        }
        const auto position = getPositionGlobal();
        const auto quat = normalize(toQuat(mat3(worldTransform)));
        const auto pos = JPH::RVec3(position.x, position.y + yDelta, position.z);
        const auto rot = JPH::Quat(quat.x, quat.y, quat.z, quat.w);
        virtualCharacter->SetPosition(pos);
        virtualCharacter->SetRotation(rot);
        physicsCharacter->SetPositionAndRotation(pos, rot);
    }

    void Character::_physicsUpdate(const float delta) {
        Node::_physicsUpdate(delta);
        if (bodyId.IsInvalid() || !bodyInterface.IsAdded(bodyId)) { return; }
        physicsCharacter->PostSimulation(0.01f);
        virtualCharacter->Update(delta,
              virtualCharacter->GetUp() * Application::get()._getPhysicsSystem().GetGravity().Length(),
              *this,
              *this,
              *this,
              {},
              *Application::get()._getTempAllocator().get());
    }

    void Character::_update(const float alpha) {
        Node::_update(alpha);
        if (bodyId.IsInvalid() || !bodyInterface.IsAdded(bodyId)) { return; }
        updating = true;
        JPH::Vec3 position;
        JPH::Quat rotation;
        physicsCharacter->GetPositionAndRotation( position, rotation);
        const auto pos = vec3{position.GetX(), position.GetY() - yDelta, position.GetZ()};
        if (pos != getPositionGlobal()) {
            setPositionGlobal(pos);
        }
        const auto rot = quat{rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ()};
        if (rot != getRotationQuaternion()) {
            setRotation(rot);
        }
        updating = false;
    }

    void Character::OnContactAdded(const JPH::CharacterVirtual *  inCharacter,
                                   const JPH::BodyID &            inBodyID2,
                                   const JPH::SubShapeID &        inSubShapeID2,
                                   JPH::RVec3Arg                  inContactPosition,
                                   JPH::Vec3Arg                   inContactNormal,
                                   JPH::CharacterContactSettings &ioSettings) {
        const auto *charac = reinterpret_cast<Character *>(inCharacter->GetUserData());
        auto *      node   = reinterpret_cast<CollisionObject *>(bodyInterface.GetUserData(inBodyID2));
        assert(charac && node && "physics body not associated with a node");
        auto event = Collision{
                .position = vec3{inContactPosition.GetX(), inContactPosition.GetY(), inContactPosition.GetZ()},
                .normal = vec3{inContactNormal.GetX(), inContactNormal.GetY(), inContactNormal.GetZ()},
                .object = node
        };
        this->emit(on_collision_starts, &event);
    }

    bool Character::ShouldCollide(const JPH::ObjectLayer inLayer) const {
        const auto targetLayer = (inLayer >> PHYSICS_LAYERS_BITS) & PHYSICS_LAYERS_MASK;
        // log("Character::ShouldCollide", to_string(targetLayer), to_string(collisionMask), to_string((targetLayer & collisionMask) != 0));
        return (targetLayer & collisionMask) != 0;
    }

    bool Character::ShouldCollide(const JPH::BodyID &inBodyID) const {
        const auto node1 = reinterpret_cast<CollisionObject *>(bodyInterface.GetUserData(inBodyID));
        // log("Character::ShouldCollide", getName(), node1->getName());
        return (node1->getCollisionLayer() & collisionMask) != 0;
    }

    bool Character::ShouldCollideLocked(const JPH::Body &inBody) const {
        const auto node1 = reinterpret_cast<CollisionObject *>(inBody.GetUserData());
        // log("Character::ShouldCollideLocked", getName(), node1->getName());
        return (node1->getCollisionLayer() & collisionMask) != 0;
    }

    void Character::_onEnterScene() {
        if (isProcessed() && !bodyId.IsInvalid()) {
            if (!bodyInterface.IsAdded(bodyId)) {
                physicsCharacter->AddToPhysicsSystem();
                Application::get()._setOptimizeBroadPhase();
            }
            bodyInterface.SetObjectLayer(bodyId, collisionLayer << PHYSICS_LAYERS_BITS | collisionMask);
            setPositionAndRotation();
        }
        Node::_onEnterScene();
    }

    void Character::_onExitScene() {
        if (isProcessed() && !bodyId.IsInvalid() && bodyInterface.IsAdded(bodyId)) {
            physicsCharacter->RemoveFromPhysicsSystem();
        }
        Node::_onExitScene();
    }

    void Character::_onPause() {
        if (isProcessed()  && !bodyId.IsInvalid() && bodyInterface.IsAdded(bodyId)) {
            physicsCharacter->RemoveFromPhysicsSystem();
        }
    }

    void Character::_onResume() {
        if (isProcessed() && !bodyId.IsInvalid()) {
            if (visible) {
                if (!bodyInterface.IsAdded(bodyId)) {
                    physicsCharacter->AddToPhysicsSystem();
                    Application::get()._setOptimizeBroadPhase();
                }
                bodyInterface.SetObjectLayer(bodyId, collisionLayer << PHYSICS_LAYERS_BITS | collisionMask);
                setPositionAndRotation();
            }
        }
    }

    void Character::setVisible(const bool visible) {
        if (!bodyId.IsInvalid() && visible != this->visible) {
            if (visible) {
                if (!bodyInterface.IsAdded(bodyId)) {
                    physicsCharacter->AddToPhysicsSystem();
                    Application::get()._setOptimizeBroadPhase();
                }
                bodyInterface.SetObjectLayer(bodyId, collisionLayer << PHYSICS_LAYERS_BITS | collisionMask);
                setPositionAndRotation();
            } else {
                if (bodyInterface.IsAdded(bodyId)) {
                    physicsCharacter->RemoveFromPhysicsSystem();
                }
            }
        }
        Node::setVisible(visible);
    }

}