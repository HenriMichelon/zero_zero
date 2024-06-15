#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/resources/image.h"
#include "z0/resources/texture.h"
#include "z0/resources/material.h"
#include "z0/resources/mesh.h"
#include "z0/resources/shape.h"
#include "z0/nodes/collision_object.h"
#include "z0/nodes/character.h"
#include "z0/application.h"
#endif

namespace z0 {

    Character::Character(shared_ptr<Shape> shape,
                         uint32_t layer,
                         uint32_t mask,
                         const string& name):
            CollisionObject(shape,
                        layer,
                        mask,
                        name) {
        auto position = getPositionGlobal();
        auto quat = normalize(toQuat(mat3(worldTransform)));
        // TODO : use a capsule shape
        auto shapeHe = reinterpret_cast<JPH::BoxShapeSettings*>(shape->_getShapeSettings())->mHalfExtent;
        auto pos = JPH::RVec3(position.x, position.y, position.z);
        auto rot = JPH::Quat(quat.x, quat.y, quat.z, quat.w);

        JPH::CharacterVirtualSettings settingsVirtual;
        settingsVirtual.mShape = new JPH::BoxShape(shapeHe);
        settingsVirtual.mMaxSlopeAngle = radians(45.0);
        character = make_unique<JPH::CharacterVirtual>(&settingsVirtual,
                                                pos,
                                                rot,
                                                0,
                                                &app()._getPhysicsSystem());
    	character->SetUp(JPH::Vec3{upVector.x, upVector.y, upVector.z});
        character->SetUserData(reinterpret_cast<uint64>(this));
        character->SetListener(this);

        JPH::CharacterSettings settings;
        settings.mLayer = collisionLayer << 4 | collisionMask;
        settings.mShape = new JPH::BoxShape(shapeHe * 0.90);
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

    void Character::setUpVector(vec3 v) {
        upVector = v;
        character->SetUp(JPH::Vec3{upVector.x, upVector.y, upVector.z});
    }

    bool Character::isOnGround() {
        return character->GetGroundState() == JPH::CharacterBase::EGroundState::OnGround;
    }

    bool Character::isGround(CollisionObject *node) {
        return node->_getBodyId() == character->GetGroundBodyID();
    }

    void Character::setPositionAndRotation() {
        if (updating || (parent == nullptr)) return;
        auto pos = getPositionGlobal();
        auto quat = normalize(toQuat(mat3(worldTransform)));
        auto jpos = JPH::RVec3(pos.x, pos.y, pos.z);
        auto jquat = JPH::Quat(quat.x, quat.y, quat.z, quat.w);
        character->SetPosition(jpos);
        character->SetRotation(jquat);
        physicsCharacter->SetPositionAndRotation(jpos, jquat);
    }

    void Character::setVelocity(vec3 velocity) {
        if (velocity == VEC3ZERO) {
            character->SetLinearVelocity(JPH::Vec3::sZero());
        } else {
            // current orientation * velocity
            velocity = toQuat(mat3(localTransform)) * velocity;
            character->SetLinearVelocity(JPH::Vec3{velocity.x, velocity.y, velocity.z});
        }
    }

    vec3 Character::getVelocity() const {
        auto velocity = character->GetLinearVelocity();
        return vec3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
    }

    vec3 Character::getGroundVelocity() const {
        auto velocity = character->GetGroundVelocity();
        return vec3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
    }

    void Character::_physicsUpdate(float delta) {
        updating = true;
        character->Update(delta, 
                          character->GetUp() * app()._getPhysicsSystem().GetGravity().Length(), 
                          *this,
                          *this,
                          *this,
                          {}, 
                          *app()._getTempAllocator().get());
        auto pos = character->GetPosition();
        auto newPos = vec3{pos.GetX(), pos.GetY(), pos.GetZ()};
        if (newPos != getPositionGlobal()) {
            setPositionGlobal(newPos);
            bodyInterface.MoveKinematic(physicsCharacter->GetBodyID(), pos,  character->GetRotation(), delta);
        }
        /*for(const auto& contact : character->GetActiveContacts()) {
            if (contact.mBodyB != character->GetGroundBodyID()) {
                bodyInterface.AddForce(contact.mBodyB, JPH::Vec3{0.0, 0.0, -10000.0}, contact.mPosition);
            }
        }*/
        updating = false;
    }

    list<Character::Collision> Character::getCollisions() const {
        list<Character::Collision> contacts;
        for(const auto& contact : character->GetActiveContacts()) {
            auto* node = reinterpret_cast<CollisionObject*>(bodyInterface.GetUserData(contact.mBodyB));
            assert(node && "physics body not associated with a node");
            contacts.push_back({
                vec3{contact.mPosition.GetX(), contact.mPosition.GetY(), contact.mPosition.GetZ()},
                vec3{contact.mSurfaceNormal.GetX(), contact.mSurfaceNormal.GetY(), contact.mSurfaceNormal.GetZ()},
                node
                });
        }
        return contacts;
    }

    void Character::OnContactAdded(const JPH::CharacterVirtual *inCharacter, 
                            const JPH::BodyID &inBodyID2, 
                            const JPH::SubShapeID &inSubShapeID2, 
                            JPH::RVec3Arg inContactPosition, 
                            JPH::Vec3Arg inContactNormal, 
                            JPH::CharacterContactSettings &ioSettings) {
        auto* charac = reinterpret_cast<Character*>(inCharacter->GetUserData());
        auto* node = reinterpret_cast<CollisionObject*>(bodyInterface.GetUserData(inBodyID2));
        assert(charac && node && "physics body not associated with a node");
        charac->onCollisionStarts({
            vec3{inContactPosition.GetX(), inContactPosition.GetY(), inContactPosition.GetZ()},
            vec3{inContactNormal.GetX(), inContactNormal.GetY(), inContactNormal.GetZ()},
            node
        });        
    }

    bool Character::ShouldCollide (JPH::ObjectLayer inLayer) const {
        auto targetLayer = (inLayer >> 4) & 0b1111;
        return (targetLayer & collisionMask) != 0;
    }

    bool Character::ShouldCollide (const JPH::BodyID &inBodyID) const {
        auto node1 = reinterpret_cast<CollisionObject*>(bodyInterface.GetUserData(inBodyID));
        return (node1->getCollisionLayer() & collisionMask) != 0;
    }

    bool Character::ShouldCollideLocked (const JPH::Body &inBody) const {
        auto node1 = reinterpret_cast<CollisionObject*>(inBody.GetUserData());
        return (node1->getCollisionLayer() & collisionMask) != 0;
    }

}
