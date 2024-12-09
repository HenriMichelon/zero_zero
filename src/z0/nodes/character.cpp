/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Collision/ObjectLayerPairFilterMask.h>
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
                         const string & name):
        CollisionObject(layer,
                        name,
                        CHARACTER) {
        setShape(height, radius);
    }

    void Character::setShape(const float height, const float radius) {
        assert(height/2 > radius);
        yDelta = height/2;
        const auto position= getPositionGlobal();
        const auto quat = normalize(getRotationQuaternion());
        const auto pos = JPH::RVec3(position.x, position.y + yDelta, position.z);
        const auto rot = JPH::Quat(quat.x, quat.y, quat.z, quat.w);

        JPH::CharacterVirtualSettings settingsVirtual;
        settingsVirtual.mShape          = new JPH::CapsuleShape(height/2 - radius, radius);
        settingsVirtual.mInnerBodyLayer = collisionLayer;
        settingsVirtual.mInnerBodyShape = new JPH::CapsuleShape(height/2 - radius, radius);
        // settingsVirtual.mMaxSlopeAngle  = radians(25.0);
        settingsVirtual.mEnhancedInternalEdgeRemoval = true;
        settingsVirtual.mUp = JPH::Vec3{upVector.x, upVector.y, upVector.z};
        virtualCharacter = make_unique<JPH::CharacterVirtual>(&settingsVirtual,
                                                       pos,
                                                       rot,
                                                       reinterpret_cast<uint64>(this),
                                                       &app()._getPhysicsSystem());
        setCollisionLayer(collisionLayer);
        virtualCharacter->SetListener(this);
    }

    vec3 Character::getGroundVelocity() const {
        const auto velocity = virtualCharacter->GetGroundVelocity();
        return vec3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
    }

    void Character::setUpVector(const vec3 vector) {
        upVector = vector;
        virtualCharacter->SetUp(JPH::Vec3{upVector.x, upVector.y, upVector.z});
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
        if (velocity == VEC3ZERO) {
            virtualCharacter->SetLinearVelocity(JPH::Vec3::sZero());
        } else {
            // current orientation * velocity
            const auto vel = getRotationQuaternion() * velocity;
            virtualCharacter->SetLinearVelocity(JPH::Vec3{vel.x, vel.y, vel.z});
        }
    }

    void Character::setPositionAndRotation() {
        if (updating) {
            return;
        }
        const auto position = getPositionGlobal();
        const auto quat = normalize(toQuat(mat3(worldTransform)));
        const auto pos = JPH::RVec3(position.x, position.y + yDelta, position.z);
        const auto rot = JPH::Quat(quat.x, quat.y, quat.z, quat.w);
        virtualCharacter->SetPosition(pos);
        virtualCharacter->SetRotation(rot);
    }

    void Character::_physicsUpdate(const float delta) {
        Node::_physicsUpdate(delta);
        virtualCharacter->Update(delta,
              virtualCharacter->GetUp() * app()._getPhysicsSystem().GetGravity().Length(),
              *this,
              *objectLayerFilter,
              *this,
              {},
              *app()._getTempAllocator().get());
    }

    void Character::_update(const float alpha) {
        Node::_update(alpha);
        updating = true;
        const auto position = virtualCharacter->GetPosition();
        const auto pos = vec3{position.GetX(), position.GetY() - yDelta, position.GetZ()};
        if (pos != getPositionGlobal()) {
            setPositionGlobal(pos);
        }
        const auto rotation = virtualCharacter->GetRotation();
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
        auto *node   = reinterpret_cast<CollisionObject *>(bodyInterface.GetUserData(inBodyID2));
        assert(node && "physics body not associated with a node");
        auto event = Collision{
            .position = vec3{inContactPosition.GetX(), inContactPosition.GetY(), inContactPosition.GetZ()},
            .normal = vec3{inContactNormal.GetX(), inContactNormal.GetY(), inContactNormal.GetZ()},
            .object = node
        };
        // log("Character::OnContactAdded", on_collision, node->getName());
        app().callDeferred([this, event]{
            this->emit(on_collision, (void*)&event);
        });
    }

    bool Character::OnContactValidate(const JPH::CharacterVirtual *  inCharacter,
                                    const JPH::BodyID &            inBodyID2,
                                    const JPH::SubShapeID &        inSubShapeID2) {
        const auto *node = reinterpret_cast<CollisionObject *>(bodyInterface.GetUserData(inBodyID2));
        return isProcessed() && node->isProcessed();
    }

    bool Character::ShouldCollide(const JPH::BodyID &inBodyID) const {
        if (!isProcessed()) { return false; }
        const auto node1 = reinterpret_cast<CollisionObject *>(bodyInterface.GetUserData(inBodyID));
        return objectLayerFilter->ShouldCollide(node1->getCollisionLayer());
    }

    bool Character::ShouldCollideLocked(const JPH::Body &inBody) const {
        if (!isProcessed()) { return false; }
        const auto node1 = reinterpret_cast<CollisionObject *>(inBody.GetUserData());
        return objectLayerFilter->ShouldCollide(node1->getCollisionLayer());
    }

    vec3 Character::getVelocity() const {
        if (bodyId.IsInvalid()) { return VEC3ZERO; }
        const auto velocity = virtualCharacter->GetLinearVelocity();
        return vec3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
    }

    void Character::setCollisionLayer(const uint32_t layer) {
        collisionLayer = layer;
        objectLayerFilter = make_unique<JPH::DefaultObjectLayerFilter>(
            app()._getObjectLayerPairFilter(),
            collisionLayer);
    }

    void Character::_onEnterScene() {
        setPositionAndRotation();
        Node::_onEnterScene();
    }

    void Character::_onExitScene() {
        Node::_onExitScene();
    }

    void Character::_onResume() {
        setPositionAndRotation();
    }

    void Character::setVisible(const bool visible) {
        if (visible != isVisible() && visible) {
            setPositionAndRotation();
        }
        Node::setVisible(visible);
    }

}