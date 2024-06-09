#pragma once

namespace z0 {

    class CollisionNode: public Node {
    public:
        ~CollisionNode() override = default;

        uint32_t getCollisionLayer() const { return collisionLayer; }
        uint32_t getCollistionMask() const { return collisionMask; }
        bool haveCollisionLayer(uint32_t layer) const;
        bool haveCollisionMask(uint32_t layer) const;
        virtual void setCollistionLayer(uint32_t layer, bool value);
        virtual void setCollistionMask(uint32_t layer, bool value);
        bool shouldCollide(uint32_t layer) const;

    protected:
        uint32_t collisionLayer;
        uint32_t collisionMask;
        JPH::BodyInterface& bodyInterface;

        CollisionNode(uint32_t layer,
                      uint32_t mask,
                      const string& name);
        CollisionNode* _getByBodyId(JPH::BodyID id);
    };


}