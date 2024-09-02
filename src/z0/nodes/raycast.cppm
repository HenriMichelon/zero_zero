module;
#include "z0/jolt.h"
#include "z0/modules.h"
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>

export module Z0:RayCast;

import :CollisionObject;
import :Node;

export namespace z0 {

    /**
     * A ray in 3D space, used to find the first CollisionObject it intersects.
     */
    class RayCast : public Node, public JPH::ObjectLayerFilter, public JPH::BodyFilter  {
    public:
        /**
         * Creates a RayCast
         * @param target The ray's destination point, relative to the RayCast's position
         * @param mask The ray's collision mask. Only objects in at least one collision layer enabled in the mask will be detected
         * @param name The node's name
         */
        RayCast(vec3 target, uint32_t mask, const string& name = "RayCast");

        /**
         * Returns whether any object is intersecting with the ray's vector (considering the vector length).
         */
        [[nodiscard]] bool isColliding() const;

        /**
         * Returns the first object that the ray intersects, or `nullptr` if no object is intersecting the ray
         */
        [[nodiscard]] CollisionObject* getCollider() const;

        /**
         * Returns the collision point at which the ray intersects the closest object, in the global coordinate system
         */
        [[nodiscard]] vec3 getCollisionPoint() const;

        /**
         * If `true`, collisions will be ignored for this RayCast's immediate parent.
         */
        void setExcludeParent(bool);

        /**
         * Updates the collision information for the ray immediately, 
         * without waiting for the next physics update
         */
        void forceRaycastUpdate();

        /**
         * Sets the ray's destination point, relative to the RayCast's position.
         */
        void setTarget(vec3);

    private:
        vec3 target;
        vec3 hitPoint;
        uint32_t collisionMask;
        bool excludeParent{true};
        CollisionObject* collider{nullptr};
        JPH::BroadPhaseLayerFilter broadPhaseLayerFilter{};

    public:
        void _physicsUpdate(float delta) override;
        bool ShouldCollide (JPH::ObjectLayer inLayer) const override;
        bool ShouldCollideLocked (const JPH::Body &inBody) const override;
    };

     RayCast::RayCast(vec3 _target,
                     uint32_t mask,
                     const string& name):
        Node(name),
        target{_target},
        collisionMask{mask} {
    }

    bool RayCast::isColliding() const {
        return collider != nullptr;
    }

    CollisionObject* RayCast::getCollider() const {
        return collider;
    }

    vec3 RayCast::getCollisionPoint() const {
        return hitPoint;
    }

    bool RayCast::ShouldCollide (JPH::ObjectLayer inLayer) const {
        auto targetLayer = (inLayer >> 4) & 0b1111;
        return (targetLayer & collisionMask) != 0;
    }

    bool RayCast::ShouldCollideLocked (const JPH::Body &inBody) const {
        auto* node = reinterpret_cast<CollisionObject*>(inBody.GetUserData());
        return (node != nullptr) && (!(excludeParent && (node == parent)));
    }

    void RayCast::setExcludeParent(bool exclude) {
        excludeParent = exclude;
    }

    void RayCast::setTarget(vec3 t) {
        target = t;
    }

    void RayCast::forceRaycastUpdate() {
        _physicsUpdate(0.0f);
    }

    void RayCast::_physicsUpdate(float delta) {
        Node::_physicsUpdate(delta);
        const auto position = getPositionGlobal();
        const auto worldTarget = toGlobal(target);
        const JPH::RRayCast ray{
            JPH::Vec3{position.x, position.y, position.z},
            JPH::Vec3{worldTarget.x, worldTarget.y, worldTarget.z}
        };
        JPH::RayCastResult result;
        if (app()._getPhysicsSystem().GetNarrowPhaseQuery().CastRay(ray, result, broadPhaseLayerFilter, *this, *this)) {
            collider = reinterpret_cast<CollisionObject*>(app()._getBodyInterface().GetUserData(result.mBodyID));
            auto posInRay = ray.GetPointOnRay(result.mFraction);
            hitPoint = vec3{posInRay.GetX(), posInRay.GetY(), posInRay.GetZ()};
        } else {
            collider = nullptr;
        }
    }

}
