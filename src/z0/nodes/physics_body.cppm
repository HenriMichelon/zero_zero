/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/EActivation.h>
#include "z0/libraries.h"

export module z0.PhysicsBody;

import z0.CollisionObject;
import z0.Shape;

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

    protected:
        PhysicsBody(const shared_ptr<Shape> &shape,
                    uint32_t                 layer,
                    uint32_t                 mask,
                    JPH::EActivation         activationMode,
                    JPH::EMotionType         motionType,
                    const string &           name= TypeNames[PHYSICS_BODY],
                    Type                     type = PHYSICS_BODY);

        PhysicsBody(uint32_t         layer,
                    uint32_t         mask,
                    JPH::EActivation activationMode,
                    JPH::EMotionType motionType,
                    const string &   name = TypeNames[PHYSICS_BODY],
                    Type             type = PHYSICS_BODY);

        /**
         * Sets a new collision shape, recreates the body in the physic system
         */
        void setShape(const shared_ptr<Shape> &shape);

        void setProperty(const string &property, const string &value) override;

        void recreateBody();

    private:
        JPH::EMotionType motionType;
    };

}
