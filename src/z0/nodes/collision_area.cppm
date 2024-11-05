/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/jolt.h"
#include "z0/libraries.h"

export module z0.CollisionArea;

import z0.CollisionObject;
import z0.Shape;

export namespace z0 {

    /**
     * Collision sensor that reports contacts with other bodies.
     */
    class CollisionArea : public CollisionObject {
    public:
        /**
         * Creates a CollisionArea using the given geometric `shape`
         * to detect collision with bodies having a layer in the `mask` value.
         */
        CollisionArea(const shared_ptr<Shape> &shape,
                      uint32_t                 mask,
                      const string &           name = TypeNames[COLLISION_AREA]);

        /**
         * Creates a CollisionArea without collision shape
         */
        explicit CollisionArea(const string &name = TypeNames[COLLISION_AREA]);

        void setShape(const shared_ptr<Shape> &shape);

        ~CollisionArea() override;

    };

}
