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
#include "z0/libraries.h"

export module z0.nodes.Character;

import z0.Constants;

import z0.nodes.CollisionObject;

import z0.resources.Shape;

export namespace z0 {

    /**
     * %A 3D physics body specialized for characters moved by code
     */
    class Character : public CollisionObject,
                      public JPH::BroadPhaseLayerFilter,
                      public JPH::ObjectLayerFilter,
                      public JPH::BodyFilter,
                      public JPH::CharacterContactListener {
    public:
        /**
         * Creates a Character with a given collision `shape`,
         * belonging to the `layer` layers and detecting collisions
         * with bodies having a layer in the `mask` value.
         */
        explicit Character(float      height,
                           float      radius,
                           uint32_t      layer,
                           uint32_t      mask,
                           const string &name = TypeNames[CHARACTER]);

        /**
         * Creates a Character without a collision `shape`,
         */
        // explicit Character(const string &name = TypeNames[CHARACTER]);

        /**
         * Sets a new capsule shape, recreates the virtualCharacter in the physic system
         */
        void setShape(float height, float radius);

        ~Character() override;

        /**
         * Returns `true` if the Character is on a ground
         */
        [[nodiscard]] inline bool isOnGround() const {
            return virtualCharacter->GetGroundState() == JPH::CharacterBase::EGroundState::OnGround;
        }

        /**
         * Returns `true` if `object` is the ground
         */
        [[nodiscard]] inline bool isGround(const CollisionObject *object) const {
            return object->_getBodyId() == virtualCharacter->GetGroundBodyID();
        }

        /**
         * Returns the velocity in the world space of the ground.
         */
        [[nodiscard]] vec3 getGroundVelocity() const;

        /**
         * Returns the UP axis for this Character
         */
        [[nodiscard]] inline const vec3 &getUpVector() const { return upVector; }

        /**
         * Sets the UP axis for this Character
         */
        void setUpVector(vec3 vector);

        /**
        * Returns the list of the currently colliding bodies
        */
        [[nodiscard]] list<Collision> getCollisions() const;

        /**
         * Moves the virtualCharacter using this velocity
         */
        void setVelocity(vec3 velocity) override;

        /**
         * Returns the current virtualCharacter velocity
         */
        // [[nodiscard]] vec3 getVelocity() const override;

        inline float getHeight() const { return height; }

        inline float getRadius() const { return radius; }

    protected:
        void setPositionAndRotation() override;

    private:
        float height;
        float radius;
        float yDelta; // https://jrouwe.github.io/JoltPhysics/class_character_base_settings.html#aee9be06866efe751ab7e2df57edee6b1
        vec3                              upVector{AXIS_UP};
        unique_ptr<JPH::CharacterVirtual> virtualCharacter;
        unique_ptr<JPH::Character>        physicsCharacter;

    public:
        void _physicsUpdate(float delta) override;

        void OnContactAdded(const JPH::CharacterVirtual *  inCharacter,
                            const JPH::BodyID &            inBodyID2,
                            const JPH::SubShapeID &        inSubShapeID2,
                            JPH::RVec3Arg                  inContactPosition,
                            JPH::Vec3Arg                   inContactNormal,
                            JPH::CharacterContactSettings &ioSettings) override;

        inline bool ShouldCollide(const JPH::BroadPhaseLayer inLayer) const override {
            return true;
        }

        bool ShouldCollide(JPH::ObjectLayer inLayer) const override;

        bool ShouldCollide(const JPH::BodyID &inBodyID) const override;

        bool ShouldCollideLocked(const JPH::Body &inBody) const override;
    };

}
