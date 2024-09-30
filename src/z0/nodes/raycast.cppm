module;
#include "z0/jolt.h"
#include "z0/libraries.h"
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>

export module Z0:RayCast;

import :CollisionObject;
import :Node;

export namespace z0 {

    /**
     * A ray in 3D space, used to find the first CollisionObject it intersects.
     */
    class RayCast : public Node, public JPH::ObjectLayerFilter, public JPH::BodyFilter {
    public:
        /**
         * Creates a RayCast
         * @param target The ray's destination point, relative to the RayCast's position
         * @param mask The ray's collision mask. Only objects in at least one collision layer enabled in the mask will be detected
         * @param name The node's name
         */
        RayCast(const vec3 &target, const uint32_t mask, const string &name = "RayCast"):
            Node{name, RAYCAST},
            target{target},
            collisionMask{mask} {
        }

        /**
         * Creates a RayCast with a [0.0, 0.0, 0.0] target
         * @param name The node's name
         */
        explicit RayCast(const string &name = "RayCast"):
            Node{name, RAYCAST} {
        }

        /**
         * Returns whether any object is intersecting with the ray's vector (considering the vector length).
         */
        [[nodiscard]] bool isColliding() const {
            return collider != nullptr;
        }

        /**
         * Returns the first object that the ray intersects, or `nullptr` if no object is intersecting the ray
         */
        [[nodiscard]] CollisionObject *getCollider() const {
            return collider;
        }

        /**
         * Returns the collision point at which the ray intersects the closest object, in the global coordinate system
         */
        [[nodiscard]] vec3 getCollisionPoint() const {
            return hitPoint;
        }

        /**
         * If `true`, collisions will be ignored for this RayCast's immediate parent.
         */
        void setExcludeParent(const bool exclude) {
            excludeParent = exclude;
        }

        /**
         * Updates the collision information for the ray immediately, 
         * without waiting for the next physics update
         */
        void forceRaycastUpdate() {
            _physicsUpdate(0.0f);
        }

        /**
         * Sets the ray's destination point, relative to the RayCast's position.
         */
        void setTarget(const vec3 &target) {
            this->target = target;
        }

    private:
        vec3                       target{};
        vec3                       hitPoint{};
        uint32_t                   collisionMask{};
        bool                       excludeParent{true};
        CollisionObject *          collider{nullptr};
        JPH::BroadPhaseLayerFilter broadPhaseLayerFilter{};

    public:
        void _physicsUpdate(const float delta) override {
            Node::_physicsUpdate(delta);
            const auto          position    = getPositionGlobal();
            const auto          worldTarget = toGlobal(target);
            const JPH::RRayCast ray{
                    JPH::Vec3{position.x, position.y, position.z},
                    JPH::Vec3{worldTarget.x, worldTarget.y, worldTarget.z}
            };
            JPH::RayCastResult result;
            if (app()._getPhysicsSystem().GetNarrowPhaseQuery().CastRay(
                    ray,
                    result,
                    broadPhaseLayerFilter,
                    *this,
                    *this)) {
                collider = reinterpret_cast<CollisionObject *>(app()._getBodyInterface().GetUserData(result.mBodyID));
                auto posInRay = ray.GetPointOnRay(result.mFraction);
                hitPoint = vec3{posInRay.GetX(), posInRay.GetY(), posInRay.GetZ()};
            } else {
                collider = nullptr;
            }
        }

        bool ShouldCollide(const JPH::ObjectLayer inLayer) const override {
            const auto targetLayer = (inLayer >> 4) & 0b1111;
            return (targetLayer & collisionMask) != 0;
        }

        bool ShouldCollideLocked(const JPH::Body &inBody) const override {
            const auto *node = reinterpret_cast<CollisionObject *>(inBody.GetUserData());
            return (node != nullptr) && (!(excludeParent && (node == parent)));
        }

    };

}
