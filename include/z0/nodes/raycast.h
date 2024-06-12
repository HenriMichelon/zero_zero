#pragma once

namespace z0 {

    class RayCast : public Node, public JPH::ObjectLayerFilter, public JPH::BodyFilter  {
    public:
        RayCast(vec3 target, uint32_t mask, const string& name = "RayCast");

        bool isColliding() const;
        CollisionObject* getCollider() const;
        vec3 getCollisionPoint() const;
        void setExcludeParent(bool);
        void forceRaycastUpdate();
        void setTarget(vec3);

    private:
        vec3 target;
        vec3 hitPoint;
        uint32_t collisionMask;
        bool excludeParent{true};
        CollisionObject* collider{nullptr};
        JPH::BroadPhaseLayerFilter broadPhaseLayerFilter{};

    public:
        void _physicsUpdate() override;
        bool ShouldCollide (JPH::ObjectLayer inLayer) const override;
        bool ShouldCollideLocked (const JPH::Body &inBody) const override;
    };

}
