module;
#include "z0/jolt.h"
#include "z0/libraries.h"

export module z0:KinematicBody;

import :PhysicsBody;
import :Shape;

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
                               const string &           name  = "KinematicBody");

        /**
         * Creates a KinematicBody without a collision shape,
         */
        explicit KinematicBody(const string &name = "KinematicBody");

        ~KinematicBody() override = default;

    };

}
