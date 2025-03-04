/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include "z0/libraries.h"

export module z0.nodes.RigidBody;

import z0.nodes.PhysicsBody;

import z0.resources.Shape;

export namespace z0 {
    /**
     * %A 3D physics body that is moved by a physics simulation, responds to forces.
     */
    class RigidBody : public PhysicsBody {
    public:
        /**
         * Creates a RigidBody with a given collision `shape`, 
         * belonging to the `layer` layers and detecting collisions 
         * with bodies having a layer in the `mask` value.
         */
        explicit RigidBody(const shared_ptr<Shape> &shape,
                           uint32_t                 layer = 0,
                           const string &           name  = TypeNames[RIGID_BODY]);

        /**
         * Creates a RigidBody without a collision shape,
         */
        explicit RigidBody(const string &name = TypeNames[RIGID_BODY]);

        ~RigidBody() override = default;

        void setProperty(const string &property, const string &value) override;

    protected:
        shared_ptr<Node> duplicateInstance() const override;
    };
}
