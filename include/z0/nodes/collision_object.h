#pragma once

namespace z0 {

    /**
     * Base class for 3D physics objects
     */
    class CollisionObject: public Node {
    public:
        struct Collision {
            vec3             position;
            vec3             normal;
            CollisionObject* object;
        };

        uint32_t getCollisionLayer() const { return collisionLayer; }
        uint32_t getCollistionMask() const { return collisionMask; }
        bool haveCollisionLayer(uint32_t layer) const;
        bool haveCollisionMask(uint32_t layer) const;
        virtual void setCollistionLayer(uint32_t layer, bool value);
        virtual void setCollistionMask(uint32_t layer, bool value);
        bool shouldCollide(uint32_t layer) const;
        virtual void onCollisionStarts(const Collision collision) {};
        virtual void setVelocity(vec3 velocity);
        virtual vec3 getVelocity() const;
        void _updateTransform() override;
        void _updateTransform(const mat4& parentMatrix) override;
        void applyForce(vec3 force);
        void applyForce(vec3 force, vec3 point);

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
        ~CollisionObject() override = default;
    };


}