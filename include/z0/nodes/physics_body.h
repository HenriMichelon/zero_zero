#pragma once

namespace z0 {

    class PhysicsBody: public CollisionObject {
    public:
        ~PhysicsBody() override;

        void setGravityScale(float value);

    protected:
        PhysicsBody(shared_ptr<Shape>& shape,
                    uint32_t layer,
                    uint32_t mask,
                    JPH::EActivation activationMode,
                    JPH::EMotionType motionType,
                    const string& name);

    private:
        JPH::EMotionType motionType;
    };

}