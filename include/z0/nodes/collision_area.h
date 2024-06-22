#pragma once

namespace z0 {

    /**
     * Collision sensor
     */
    class CollisionArea: public CollisionObject {
    public:
        /**
         * 
         */
        CollisionArea(shared_ptr<Shape> shape,
                    uint32_t layer,
                    uint32_t mask,
                    const string& name = "CollisionArea");
        ~CollisionArea() override;
    };

}