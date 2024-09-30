module;
#include "z0/jolt.h"
#include "z0/libraries.h"

export module Z0:KinematicBody;

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
                               const uint32_t           layer = 0xff,
                               const uint32_t           mask  = 0xff,
                               const string &           name  = "KinematicBody"):
            PhysicsBody(shape,
                        layer,
                        mask,
                        JPH::EActivation::Activate,
                        JPH::EMotionType::Kinematic,
                        name,
                        KINEMATIC_BODY) {
        }

        /**
         * Creates a KinematicBody without a collision shape,
         */
        explicit KinematicBody(const string &name = "KinematicBody"):
            PhysicsBody(0xff,
                        0xff,
                        JPH::EActivation::Activate,
                        JPH::EMotionType::Kinematic,
                        name,
                        KINEMATIC_BODY) {
        }

        ~KinematicBody() override = default;

    };

}
