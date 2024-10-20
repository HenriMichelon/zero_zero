module;
#include "z0/jolt.h"
#include "z0/libraries.h"
#include <glm/gtx/quaternion.hpp>
#include <Jolt/Physics/Character/Character.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

module z0;

import :Constants;
import :CollisionObject;
import :Shape;
import :Application;

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
        this->shape         = shape;
        const auto position = getPositionGlobal();
        const auto quat     = normalize(toQuat(mat3(worldTransform)));
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
                                                       0,
                                                       &app()._getPhysicsSystem());
        character->SetUp(JPH::Vec3{upVector.x, upVector.y, upVector.z});
        character->SetUserData(reinterpret_cast<uint64>(this));
        character->SetListener(this);

        JPH::CharacterSettings settings;
        settings.mLayer  = collisionLayer << 4 | collisionMask;
        settings.mShape  = new JPH::BoxShape(shapeHe * 0.90);
        physicsCharacter = make_unique<JPH::Character>(&settings,
                                                       pos,
                                                       rot,
                                                       0,
                                                       &Application::get()._getPhysicsSystem());
        bodyInterface.SetUserData(physicsCharacter->GetBodyID(), reinterpret_cast<uint64>(this));
        physicsCharacter->AddToPhysicsSystem();
    }

    Character::~Character() {
        physicsCharacter->RemoveFromPhysicsSystem();
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
        const auto velocity = character->GetLinearVelocity();
        return vec3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
    }

    void Character::setVelocity(const vec3 velocity) {
        if (velocity == VEC3ZERO) {
            character->SetLinearVelocity(JPH::Vec3::sZero());
        } else {
            // current orientation * velocity
            const auto vel = toQuat(mat3(localTransform)) * velocity;
            character->SetLinearVelocity(JPH::Vec3{vel.x, vel.y, vel.z});
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
        Node::_physicsUpdate(delta);
        updating = true;
        character->Update(delta,
                          character->GetUp() * app()._getPhysicsSystem().GetGravity().Length(),
                          *this,
                          *this,
                          *this,
                          {},
                          *app()._getTempAllocator().get());
        const auto pos    = character->GetPosition();
        const auto newPos = vec3{pos.GetX(), pos.GetY(), pos.GetZ()};
        if (newPos != getPositionGlobal()) {
            setPositionGlobal(newPos);
            bodyInterface.MoveKinematic(physicsCharacter->GetBodyID(), pos, character->GetRotation(), delta);
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
        const auto targetLayer = (inLayer >> 4) & 0b1111;
        return (targetLayer & collisionMask) != 0;
    }

    bool Character::ShouldCollide(const JPH::BodyID &inBodyID) const {
        const auto node1 = reinterpret_cast<CollisionObject *>(bodyInterface.GetUserData(inBodyID));
        return (node1->getCollisionLayer() & collisionMask) != 0;
    }

    bool Character::ShouldCollideLocked(const JPH::Body &inBody) const {
        const auto node1 = reinterpret_cast<CollisionObject *>(inBody.GetUserData());
        return (node1->getCollisionLayer() & collisionMask) != 0;
    }

}