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
#include <glm/gtx/quaternion.hpp>
#include "z0/libraries.h"

module z0.Character;

import z0.Application;
import z0.Constants;
import z0.CollisionObject;
import z0.Shape;
import z0.Tools;

namespace z0 {
    Character::Character(const shared_ptr<Shape> &shape,
                         const uint32_t           layer,
                         const uint32_t           mask,
                         const string &           name):
        CollisionObject(shape,
                        layer,
                        mask,
                        name,
                        CHARACTER) {
        setShape(shape);
    }

    Character::Character(const string &name):
        CollisionObject(0, 0, name, CHARACTER) {
    }

    void Character::setShape(const shared_ptr<Shape> &shape) {
        if (physicsCharacter) {
            physicsCharacter->RemoveFromPhysicsSystem();
        }
        this->shape = shape;
        const auto position= getPositionGlobal();
        const auto quat = normalize(toQuat(mat3(worldTransform)));
        // TODO : use a capsule shape
        const auto shapeHe = reinterpret_cast<JPH::BoxShapeSettings *>(this->shape->_getShapeSettings())->
                mHalfExtent;
        auto pos = JPH::RVec3(position.x, position.y, position.z);
        auto rot = JPH::Quat(quat.x, quat.y, quat.z, quat.w);

        JPH::CharacterVirtualSettings settingsVirtual;
        settingsVirtual.mShape         = new JPH::BoxShape(shapeHe);
        settingsVirtual.mMaxSlopeAngle = radians(45.0);
        character                      = make_unique<JPH::CharacterVirtual>(&settingsVirtual,
                                                       pos,
                                                       rot,
                                                       reinterpret_cast<uint64>(this),
                                                       &Application::get()._getPhysicsSystem());
        character->SetUp(JPH::Vec3{upVector.x, upVector.y, upVector.z});
        character->SetListener(this);

        JPH::CharacterSettings settings;
        settings.mLayer  = collisionLayer << PHYSICS_LAYERS_BITS | collisionMask;
        settings.mShape  = new JPH::BoxShape(shapeHe);
        physicsCharacter = make_unique<JPH::Character>(&settings,
                                                       pos,
                                                       rot,
                                                       reinterpret_cast<uint64>(this),
                                                       &Application::get()._getPhysicsSystem());
        physicsCharacter->AddToPhysicsSystem();
    }

    Character::~Character() {
        if (physicsCharacter) {
            physicsCharacter->RemoveFromPhysicsSystem();
        }
    }

    vec3 Character::getGroundVelocity() const {
        const auto velocity = character->GetGroundVelocity();
        return vec3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
    }

    void Character::setUpVector(const vec3 vector) {
        upVector = vector;
        character->SetUp(JPH::Vec3{upVector.x, upVector.y, upVector.z});
    }

    list<CollisionObject::Collision> Character::getCollisions() const {
        list<Collision> contacts;
        for (const auto &contact : character->GetActiveContacts()) {
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

    vec3 Character::getVelocity() const {
        const auto velocity = physicsCharacter->GetLinearVelocity();
        return vec3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
    }

    void Character::setVelocity(const vec3 velocity) {
        if (velocity == VEC3ZERO) {
            character->SetLinearVelocity(JPH::Vec3::sZero());
            physicsCharacter->SetLinearVelocity(JPH::Vec3::sZero());
        } else {
            // current orientation * velocity
            const auto vel = toQuat(mat3(localTransform)) * velocity;
            character->SetLinearVelocity(JPH::Vec3{vel.x, vel.y, vel.z});
            physicsCharacter->SetLinearVelocity(JPH::Vec3{vel.x, vel.y, vel.z});
        }
    }

    void Character::setPositionAndRotation() {
        if (updating) { return; }
        const auto pos   = getPositionGlobal();
        const auto quat  = normalize(toQuat(mat3(worldTransform)));
        const auto jpos  = JPH::RVec3(pos.x, pos.y, pos.z);
        const auto jquat = JPH::Quat(quat.x, quat.y, quat.z, quat.w);
        character->SetPosition(jpos);
        character->SetRotation(jquat);
        physicsCharacter->SetPositionAndRotation(jpos, jquat);
    }

    void Character::_physicsUpdate(const float delta) {
        physicsCharacter->PostSimulation(0.01f);
        updating = true;
        character->Update(delta,
                          character->GetUp() * Application::get()._getPhysicsSystem().GetGravity().Length(),
                          *this,
                          *this,
                          *this,
                          {},
                          *Application::get()._getTempAllocator().get());
        const auto pos = character->GetPosition();
        const auto newPos = vec3{pos.GetX(), pos.GetY(), pos.GetZ()};
        if (newPos != getPositionGlobal()) {
            setPositionGlobal(newPos);
            // physicsCharacter->SetPosition(pos);
        }
        const auto rot = character->GetRotation();
        const auto newRot = quat{rot.GetW(), rot.GetX(), rot.GetY(), rot.GetZ()};
        if (newRot != getRotationQuaternion()) {
            setRotation(newRot);
            // physicsCharacter->SetRotation(rot);
        }
        updating = false;
        Node::_physicsUpdate(delta);
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

}