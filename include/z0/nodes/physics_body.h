#pragma once

namespace z0 {

    /**
     * Base class for 3D game objects affected by physics.
     */
    class PhysicsBody: public CollisionObject {
    public:
        ~PhysicsBody() override;

        /**
         * Sets an artificial gravity factor
         */
        void setGravityScale(float value);

    protected:
        PhysicsBody(shared_ptr<Shape>& shape,
                    uint32_t layer,
                    uint32_t mask,
                    JPH::EActivation activationMode,
                    JPH::EMotionType motionType,
                    const string& name);

        PhysicsBody(uint32_t layer,
                    uint32_t mask,
                    JPH::EActivation activationMode,
                    JPH::EMotionType motionType,
                    const string& name);

        void setShape(shared_ptr<Shape> shape);

    private:
        JPH::EMotionType motionType;
    };

}