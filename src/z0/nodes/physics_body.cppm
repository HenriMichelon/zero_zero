/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/EActivation.h>
#include "z0/libraries.h"

export module z0.nodes.PhysicsBody;

import z0.nodes.CollisionObject;

import z0.resources.Shape;

export namespace z0 {

    /**
     * Base class for 3D game objects affected by physics.
     */
    class PhysicsBody : public CollisionObject {
    public:
        ~PhysicsBody() override = default;

        /**
         * Sets an artificial gravity factor
         */
        void setGravityScale(float value);

        /**
         * Sets the linear velocity
         */
        virtual void setVelocity(const vec3& velocity);

        /**
         *
         * Sets the gravity multiplier (set to 0.0f to disable gravity).
         */
        virtual void setGravityFactor(float factor);

        /**
        * Sets the coefficient of restitution
        * (the ratio of the relative velocity of separation after collision to the relative velocity of approach before collision)
        */
        void setBounce(float value) const;

        /**
         * Sets the body's mass.
         */
        void setMass(float value) const;

        /**
         * Returns the linear velocity
         */
        [[nodiscard]] virtual vec3 getVelocity() const;

        /**
         * Add force (unit: N) at center of mass for the next time step, will be reset after the next physics update
         */
        void applyForce(const vec3& force) const;

        /**
         * Add force (unit: N) at `position` for the next time step, will be reset after the next physics update
         */
        void applyForce(const vec3& force, const vec3& position) const;

        void setProperty(const string &property, const string &value) override;

        void recreateBody();

    protected:
        PhysicsBody(const shared_ptr<Shape> &shape,
                    uint32_t                 layer,
                    JPH::EActivation         activationMode,
                    JPH::EMotionType         motionType,
                    const string &           name= TypeNames[PHYSICS_BODY],
                    Type                     type = PHYSICS_BODY);

        PhysicsBody(uint32_t         layer,
                    JPH::EActivation activationMode,
                    JPH::EMotionType motionType,
                    const string &   name = TypeNames[PHYSICS_BODY],
                    Type             type = PHYSICS_BODY);

        /**
         * Sets a new collision shape, recreates the body in the physic system
         */
        void setShape(const shared_ptr<Shape> &shape);

    private:
        JPH::EMotionType motionType;
    };

}
