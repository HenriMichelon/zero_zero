module;
#include "z0/jolt.h"
#include "z0/libraries.h"
#include <glm/gtx/quaternion.hpp>
#include <Jolt/Physics/Character/Character.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

export module Z0:Character;

import :Constants;
import :CollisionObject;
import :Shape;
import :Application;

export namespace z0 {

    /**
     * A 3D physics body specialized for characters moved by code
     */
    class Character: public CollisionObject, 
                     public JPH::BroadPhaseLayerFilter,
                     public JPH::ObjectLayerFilter,
                     public JPH::BodyFilter,
                     public JPH::CharacterContactListener {
    public:
        /**
         * Creates a Character with a given collision `shape`, belonging to the `layer` layers and detecting collisions with bodies having a layer in the `mask` value.
         */
        explicit Character(shared_ptr<Shape> shape,
                           uint32_t layer,
                           uint32_t mask,
                           const string& name = "Character");
        ~Character() override;

        /**
         * Returns `true` if the Character is on a ground
         */
        [[nodiscard]] bool isOnGround();

        /**
         * Returns `true` if `object` is the ground
         */
        [[nodiscard]] bool isGround(CollisionObject*object);

        /**
         * Returns the velocity in the world space of the ground.
         */
        [[nodiscard]] vec3 getGroundVelocity() const;

        /**
         * Returns the UP axis for this Character
         */
        [[nodiscard]] inline const vec3& getUpVector() const { return upVector; }

        /**
         * Sets the UP axis for this Character
         */
        void setUpVector(vec3 v);

        /**
        * Returns the list of the currently colliding bodies
        */
        [[nodiscard]] list<Collision> getCollisions() const;
        

        void setVelocity(vec3 velocity) override;
        [[nodiscard]] vec3 getVelocity() const override;

    protected:
        void setPositionAndRotation() override;

    private:
        vec3 upVector{AXIS_UP};
        unique_ptr<JPH::CharacterVirtual> character;
        unique_ptr<JPH::Character> physicsCharacter;
        
    public:
        void _physicsUpdate(float delta) override;

        void OnContactAdded(const JPH::CharacterVirtual *inCharacter, 
                            const JPH::BodyID &inBodyID2, 
                            const JPH::SubShapeID &inSubShapeID2, 
                            JPH::RVec3Arg inContactPosition, 
                            JPH::Vec3Arg inContactNormal, 
                            JPH::CharacterContactSettings &ioSettings) override;
        inline bool ShouldCollide (JPH::BroadPhaseLayer inLayer) const override {
            return true;
        };
        bool ShouldCollide (JPH::ObjectLayer inLayer) const override;
        bool ShouldCollide (const JPH::BodyID &inBodyID) const override;
        bool ShouldCollideLocked (const JPH::Body &inBody) const override;
    };


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
        if (updating) { return; }
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
        Node::_physicsUpdate(delta);
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
        updating = false;
    }

    list<Character::Collision> Character::getCollisions() const {
        list<Character::Collision> contacts;
        for(const auto& contact : character->GetActiveContacts()) {
            auto* node = reinterpret_cast<CollisionObject*>(bodyInterface.GetUserData(contact.mBodyB));
            assert(node && "physics body not associated with a node");
            contacts.push_back({
                .position = vec3{contact.mPosition.GetX(), contact.mPosition.GetY(), contact.mPosition.GetZ()},
                .normal =  vec3{contact.mSurfaceNormal.GetX(), contact.mSurfaceNormal.GetY(), contact.mSurfaceNormal.GetZ()},
                .object = node
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
         auto event = CollisionObject::Collision {
            .position = vec3{inContactPosition.GetX(), inContactPosition.GetY(), inContactPosition.GetZ()},
            .normal = vec3{inContactNormal.GetX(), inContactNormal.GetY(), inContactNormal.GetZ()},
            .object = node
        };
        this->emit(on_collision_starts, &event);
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