#pragma once

namespace z0 {

    class PhysicsNode: public CollisionNode {
    public:
        ~PhysicsNode() override = default;

        virtual void setCollistionLayer(uint32_t layer, bool value);
        virtual void setCollistionMask(uint32_t layer, bool value);

        virtual void onCollisionStarts(PhysicsNode* node) {};

        void setVelocity(vec3 velocity);
        vec3 getVelocity() const;

        void updateTransform() override;
        void updateTransform(const mat4& parentMatrix) override;

    protected:
        bool updating{false};
        std::shared_ptr<Shape> shape;
        JPH::EActivation activationMode;

        PhysicsNode(shared_ptr<Shape>& shape,
                    uint32_t layer,
                    uint32_t mask,
                    const string& name);
        void setPositionAndRotation();
        void setBodyId(JPH::BodyID id);

    private:
        JPH::BodyID bodyId;

    public:
        void _physicsUpdate() override;
        void _onEnterScene() override;
        void _onExitScene() override;

        JPH::BodyID _getBodyId() const { return bodyId; }
    };


}