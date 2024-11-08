/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include "z0/libraries.h"

export module z0.KinematicBody;

import z0.PhysicsBody;
import z0.Shape;

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
                               uint32_t                 layer = 0xff,
                               uint32_t                 mask  = 0xff,
                               const string &           name  = TypeNames[KINEMATIC_BODY]);

        /**
         * Creates a KinematicBody without a collision shape,
         */
        explicit KinematicBody(const string &name = TypeNames[KINEMATIC_BODY]);

        ~KinematicBody() override = default;

    };

}
