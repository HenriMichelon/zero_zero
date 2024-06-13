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

        JPH::CharacterVirtualSettings settingsVirtual;
        settingsVirtual.mShape = new JPH::BoxShape(reinterpret_cast<JPH::BoxShapeSettings*>(shape->_getShapeSettings())->mHalfExtent);
        settingsVirtual.mMaxSlopeAngle = radians(45.0);
        character = make_unique<JPH::CharacterVirtual>(&settingsVirtual,
                                                JPH::RVec3(position.x, position.y, position.z),
                                                JPH::Quat(quat.x, quat.y, quat.z, quat.w),
                                                0,
                                                &Application::get()._getPhysicsSystem());
    	character->SetUp(JPH::Vec3{upVector.x, upVector.y, upVector.z});

        /*JPH::CharacterSettings settings;
        settings.mLayer = collisionLayer << 4 | collisionMask;
        settings.mShape = new JPH::BoxShape(reinterpret_cast<JPH::BoxShapeSettings*>(shape->_getShapeSettings())->mHalfExtent);
        subCharacter = make_unique<JPH::Character>(&settings,
                                                JPH::RVec3(position.x, position.y, position.z),
                                                JPH::Quat(quat.x, quat.y, quat.z, quat.w),
                                                0,
                                                &Application::get()._getPhysicsSystem());
        subCharacter->AddToPhysicsSystem();
        setBodyId(subCharacter->GetBodyID());*/
    }

    Character::~Character() {
        //subCharacter->RemoveFromPhysicsSystem();
    }

    void Character::setUp(vec3 v) {
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
        auto position = getPositionGlobal();
        character->SetPosition(JPH::RVec3(position.x, position.y, position.z));
        auto quat = normalize(toQuat(mat3(worldTransform)));
        character->SetRotation(JPH::Quat(quat.x, quat.y, quat.z, quat.w));
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
        //character->PostSimulation(GROUND_COLLISION_TOLERANCE);
        updating = true;
        //character->UpdateGroundVelocity();
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
        }
        updating = false;
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
