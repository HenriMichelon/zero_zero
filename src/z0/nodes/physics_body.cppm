module;
#include "z0/jolt.h"
#include "z0/libraries.h"
#include <glm/gtx/quaternion.hpp>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

export module z0:PhysicsBody;

import :CollisionObject;
import :Shape;
import :ConvexHullShape;

export namespace z0 {

    /**
     * Base class for 3D game objects affected by physics.
     */
    class PhysicsBody : public CollisionObject {
    public:
        ~PhysicsBody() override;

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
                    const string &           name,
                    Type                     type = PHYSICS_BODY);

        PhysicsBody(uint32_t         layer,
                    uint32_t         mask,
                    JPH::EActivation activationMode,
                    JPH::EMotionType motionType,
                    const string &   name,
                    Type             type = PHYSICS_BODY);

        void setShape(const shared_ptr<Shape> &shape);

        void setProperty(const string &property, const string &value) override;

    private:
        JPH::EMotionType motionType;
    };

}
