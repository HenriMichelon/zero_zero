#pragma once

#include "z0/nodes/physics_node.h"
#include "z0/resources/shape.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/EActivation.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/Body/BodyInterface.h>

namespace z0 {

    class PhysicsBody: public PhysicsNode {
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