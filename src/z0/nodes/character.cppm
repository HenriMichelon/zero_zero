/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include "z0/libraries.h"

export module z0.nodes.Character;

import z0.Constants;
import z0.Signal;

import z0.nodes.CollisionObject;

import z0.resources.Shape;

export namespace z0 {

    /**
     * %A 3D physics body specialized for characters moved by code
     */
    class Character : public CollisionObject,
                      public JPH::BroadPhaseLayerFilter,
                      public JPH::BodyFilter,
                      public JPH::CharacterContactListener {
    public:


        /**
         * Signal called whenever the character collides with a body and reports the first contact point in a CollisionObject::Collision<br>
         */
        static inline const Signal::signal on_collision = "on_character_collision";

        /**
         * Creates a Character with a given collision `shape`,
         * belonging to the `layer` layers and detecting collisions
         * with bodies having a layer in the `mask` value.
         * @param height Height of the collision shape capsule shape
         * @param radius Radius of the collision shape capsule
         * @param layer Collision layer
         * @param name Name of the node
         */
        explicit Character(float    height,
                           float    radius,
                           uint32_t layer,
                           const string &name = TypeNames[CHARACTER]);

        /**
         * Sets a new capsule shape, recreates the virtualCharacter in the physic system
         */
        void setShape(float height, float radius);

        ~Character() override = default;

        /**
         * Returns `true` if the Character is on a ground
         */
        [[nodiscard]] inline auto isOnGround() const {
            return virtualCharacter->GetGroundState() == JPH::CharacterBase::EGroundState::OnGround;
        }

        /**
         * Returns `true` if `object` is the ground
         */
        [[nodiscard]] inline auto isGround(const CollisionObject &object) const {
            return object._getBodyId() == virtualCharacter->GetGroundBodyID();
        }

        /**
         * Returns the velocity in the world space of the ground.
         */
        [[nodiscard]] vec3 getGroundVelocity() const;

        /**
         * Returns the ground node, if any
         */
        [[nodiscard]] Node* getGround() const;

        /**
         * Returns the UP axis for this Character
         */
        [[nodiscard]] inline const auto& getUpVector() const { return upVector; }

        /**
         * Sets the UP axis for this Character
         */
        void setUpVector(const vec3& vector);

        /**
        * Returns the list of the currently colliding bodies
        */
        [[nodiscard]] list<Collision> getCollisions() const;

        /**
         * Moves the virtualCharacter using this velocity
         */
        void setVelocity(const vec3& velocity);

        /**
         * Set the maximum angle of slope that character can still walk on (degrres)
         */
        void setMaxSlopeAngle(float angle) const;

        /**
         * Returns the height of the capsule collision shape
         */
        inline float getHeight() const { return height; }

        /**
         * Returns the radius of the capsule collision shape
         */
        inline float getRadius() const { return radius; }

        /**
         * Sets the current visibility of the character.
         */
        void setVisible(bool visible = true) override;

        /**
         * Sets the current collision layer
         */
        void setCollisionLayer(uint32_t layer) override;

        /**
         * Returns the linear velocity of the character
         */
        vec3 getVelocity() const;

    protected:
        void setPositionAndRotation() override;

    private:
        float height;
        float radius;
        float yDelta; // https://jrouwe.github.io/JoltPhysics/class_character_base_settings.html#aee9be06866efe751ab7e2df57edee6b1
        vec3  upVector{AXIS_UP};
        unique_ptr<JPH::CharacterVirtual> virtualCharacter;
        unique_ptr<JPH::ObjectLayerFilter> objectLayerFilter;

    public:
        void _physicsUpdate(float delta) override;

        void _update(float alpha) override;

        void _onEnterScene() override;

        void _onExitScene() override;

        void _onResume() override;

        void OnContactAdded(const JPH::CharacterVirtual *  inCharacter,
                            const JPH::BodyID &            inBodyID2,
                            const JPH::SubShapeID &        inSubShapeID2,
                            JPH::RVec3Arg                  inContactPosition,
                            JPH::Vec3Arg                   inContactNormal,
                            JPH::CharacterContactSettings &ioSettings) override;
        bool OnContactValidate(const JPH::CharacterVirtual *  inCharacter,
                            const JPH::BodyID &            inBodyID2,
                            const JPH::SubShapeID &        inSubShapeID2) override;

        inline bool ShouldCollide(const JPH::BroadPhaseLayer inLayer) const override {
            return true;
        }

        bool ShouldCollide(const JPH::BodyID &inBodyID) const override;

        bool ShouldCollideLocked(const JPH::Body &inBody) const override;
    };

}
