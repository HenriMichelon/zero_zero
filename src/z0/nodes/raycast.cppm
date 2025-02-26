/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyFilter.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include "z0/libraries.h"

export module z0.nodes.RayCast;

import z0.nodes.CollisionObject;
import z0.nodes.Node;

export namespace z0 {

    /**
     * %A ray in 3D space, used to find the first CollisionObject it intersects.
     */
    class RayCast : public Node, public JPH::BodyFilter {
    public:
        /**
         * Creates a RayCast
         * @param target The ray's destination point, relative to the RayCast's position
         * @param layer The ray's collision layer
         * @param name The node's name
         */
        RayCast(const vec3 &target, uint32_t layer, const string &name = TypeNames[RAYCAST]);

        /**
         * Creates a RayCast with a [0.0, 0.0, 0.0] target
         * @param name The node's name
         */
        explicit RayCast(const string &name = TypeNames[RAYCAST]);

        /**
         * Returns whether any object is intersecting with the ray's vector (considering the vector length).
         */
        [[nodiscard]] inline auto isColliding() const { return collider != nullptr; }

        /**
         * Returns the first object that the ray intersects, or `nullptr` if no object is intersecting the ray
         */
        [[nodiscard]] inline auto getCollider() const { return collider; }

        /**
         * Returns the collision point at which the ray intersects the closest object, in the global coordinate system
         */
        [[nodiscard]] inline auto getCollisionPoint() const { return hitPoint; }

        /**
         * If `true`, collisions will be ignored for this RayCast's immediate parent.
         */
        inline auto setExcludeParent(const bool exclude) { excludeParent = exclude; }

        /**
         * Updates the collision information for the ray immediately, 
         * without waiting for the next physics update
         */
        inline auto forceRaycastUpdate() { _physicsUpdate(0.0f); }

        /**
         * Sets the ray's destination point, relative to the RayCast's position.
         */
        inline auto setTarget(const vec3 &target) { this->target = target; }

        /**
         * Returns the ray target
         */
        inline const auto& getTarget() const { return this->target; }

        void setCollisionLayer(uint32_t layer);

    private:
        vec3                        target{};
        vec3                        hitPoint{};
        uint32_t                    collisionLayer{};
        bool                        excludeParent{true};
        shared_ptr<CollisionObject> collider{nullptr};
        JPH::BroadPhaseLayerFilter broadPhaseLayerFilter{};
        unique_ptr<JPH::ObjectLayerFilter> objectLayerFilter;

    public:
        void _physicsUpdate(float delta) override;

        bool ShouldCollideLocked(const JPH::Body &inBody) const override;

    };

}
