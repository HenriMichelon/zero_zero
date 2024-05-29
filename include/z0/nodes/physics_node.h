#pragma once

#include "z0/nodes/node.h"
#include "z0/resources/shape.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/EActivation.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/Body/BodyInterface.h>

namespace z0 {

    class PhysicsNode: public Node {
    public:
        ~PhysicsNode() override = default;

        uint32_t getCollisionLayer() const { return collisionLayer; }
        uint32_t getCollistionMask() const { return collisionMask; }
        bool haveCollisionLayer(uint32_t layer) const;
        bool haveCollisionMask(uint32_t layer) const;
        virtual void setCollistionLayer(uint32_t layer, bool value);
        virtual void setCollistionMask(uint32_t layer, bool value);

        void setVelocity(vec3 velocity);
        vec3 getVelocity() const;

        void updateTransform() override;
        void updateTransform(const mat4& parentMatrix) override;

    protected:
        bool updating{false};
        uint32_t collisionLayer;
        uint32_t collisionMask;
        std::shared_ptr<Shape> shape;
        JPH::BodyID bodyId;
        JPH::BodyInterface& bodyInterface;
        JPH::EActivation activationMode;

        PhysicsNode(shared_ptr<Shape>& shape,
                    uint32_t layer,
                    uint32_t mask,
                    const string& name);

        void setPositionAndRotation();

    public:
        void _physicsUpdate() override;
        void _onEnterScene() override;
        void _onExitScene() override;
    };

}