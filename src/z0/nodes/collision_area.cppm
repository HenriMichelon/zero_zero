/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include "z0/libraries.h"

export module z0.nodes.CollisionArea;

import z0.nodes.CollisionObject;

import z0.resources.Shape;

export namespace z0 {

    /**
     * Collision sensor that reports contacts with other bodies.
     */
    class CollisionArea : public CollisionObject {
    public:
        /**
         * Creates a CollisionArea using the given geometric `shape`
         * to detect collision with bodies having a layer in the `mask` value.
         * @param shape The collision shape
         * @param layer The collision layer
         * @param name The node name
         */
        CollisionArea(const shared_ptr<Shape> &shape,
                      uint32_t                 layer,
                      const string &           name = TypeNames[COLLISION_AREA]);

        /**
         * Creates a CollisionArea without collision shape
         */
        explicit CollisionArea(const string &name = TypeNames[COLLISION_AREA]);

        /**
         * Sets the collision shape of the area
         */
        void setShape(const shared_ptr<Shape> &shape);

        ~CollisionArea() override = default;

        void setProperty(const string &property, const string &value) override;

    protected:
        shared_ptr<Node> duplicateInstance() const override;

    };

}
