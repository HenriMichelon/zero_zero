#pragma once

namespace z0 {

    /**
     * Collision sensor that reports contacts with other bodies.
     */
    class CollisionArea: public CollisionObject {
    public:
        /**
         * Creates a CollisionArea using the given geometric `shape` to detect collision with bodies having a layer in the `mask` value.
         */
        CollisionArea(shared_ptr<Shape> shape,
                    uint32_t mask,
                    const string& name = "CollisionArea");
        ~CollisionArea() override;
    };

}