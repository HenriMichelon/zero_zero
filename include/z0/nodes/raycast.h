#pragma once

namespace z0 {

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

}
