/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include "z0/libraries.h"

export module z0.nodes.KinematicBody;

import z0.nodes.PhysicsBody;

import z0.resources.Shape;

export namespace z0 {

    /**
     * Physics body moved by velocities only
     */
    class KinematicBody : public PhysicsBody {
    public:
        /**
         * Creates a KinematicBody with a given collision `shape`, 
         * belonging to the `layer` layers and detecting collisions 
         * with bodies having a layer in the `mask` value.
         */
        explicit KinematicBody(const shared_ptr<Shape> &shape,
                               uint32_t                 layer = 0,
                               const string &           name  = TypeNames[KINEMATIC_BODY]);

        ~KinematicBody() override = default;

    protected:
        shared_ptr<Node> duplicateInstance() override;

    };

}
