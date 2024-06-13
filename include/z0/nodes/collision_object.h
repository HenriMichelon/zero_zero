#pragma once

namespace z0 {

    class CollisionObject: public Node {
    public:
        ~CollisionObject() override = default;

        uint32_t getCollisionLayer() const { return collisionLayer; }
        uint32_t getCollistionMask() const { return collisionMask; }
        bool haveCollisionLayer(uint32_t layer) const;
        bool haveCollisionMask(uint32_t layer) const;
        virtual void setCollistionLayer(uint32_t layer, bool value);
        virtual void setCollistionMask(uint32_t layer, bool value);
        bool shouldCollide(uint32_t layer) const;

        virtual void onCollisionStarts(CollisionObject* node) {};

        virtual void setVelocity(vec3 velocity);
        virtual vec3 getVelocity() const;

        void updateTransform() override;
        void updateTransform(const mat4& parentMatrix) override;

    protected:
        bool updating{false};
        uint32_t collisionLayer;
        uint32_t collisionMask;
        shared_ptr<Shape> shape;
        JPH::EActivation activationMode;
        JPH::BodyInterface& bodyInterface;

        CollisionObject(shared_ptr<Shape>& shape,
                    uint32_t layer,
                    uint32_t mask,
                    const string& name);
        void setPositionAndRotation();
        void setBodyId(JPH::BodyID id);
        CollisionObject* _getByBodyId(JPH::BodyID id);

    private:
        JPH::BodyID bodyId;

    public:
        void _physicsUpdate(float delta) override;
        void _onEnterScene() override;
        void _onExitScene() override;
        void _onPause() override;
        void _onResume() override;

        JPH::BodyID _getBodyId() const { return bodyId; }
    };


}