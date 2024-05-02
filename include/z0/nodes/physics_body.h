#pragma once

#include "z0/nodes/node.h"
#include "z0/resources/shape.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/EActivation.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/Body/BodyInterface.h>

namespace z0 {

    class PhysicsBody: public Node {
    public:
        ~PhysicsBody() override;

        uint32_t getCollisionLayer() const { return collisionLayer; }
        uint32_t getCollistionMask() const { return collisionMask; }
        bool haveCollisionLayer(uint32_t layer) const;
        bool haveCollisionMask(uint32_t layer) const;
        void setCollistionLayer(uint32_t layer, bool value);
        void setCollistionMask(uint32_t layer, bool value);

        void updateTransform() override;
        void updateTransform(const mat4& parentMatrix) override;

    protected:
        JPH::BodyID bodyId;
        JPH::BodyInterface& bodyInterface;
        std::shared_ptr<Shape> shape;

        PhysicsBody(shared_ptr<Shape>& shape,
                    uint32_t layer,
                    uint32_t mask,
                    JPH::EActivation activationMode,
                    JPH::EMotionType motionType,
                    const string& name);

    private:
        JPH::EActivation activationMode;
        JPH::EMotionType motionType;
        uint32_t collisionLayer;
        uint32_t collisionMask;
        bool updating{false};

        void setPositionAndRotation();

    public:
        void _physicsUpdate() override;
        void _onEnterScene() override;
        void _onExitScene() override;
    };

}