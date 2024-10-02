module;
#include "z0/jolt.h"
#include "z0/libraries.h"

export module z0:StaticBody;

import :PhysicsBody;
import :Shape;

export namespace z0 {

    /**
     * A 3D physics body that can't be moved by external forces. 
     */
    class StaticBody : public PhysicsBody {
    public:
        /**
         * Creates a StaticBody with a given collision `shape`, 
         * belonging to the `layer` layers.
         */
        explicit StaticBody(const shared_ptr<Shape> &shape,
                            const uint32_t           layer,
                            const string &           name = "StaticBody"):
            PhysicsBody(shape,
                        layer,
                        0,
                        JPH::EActivation::DontActivate,
                        JPH::EMotionType::Static,
                        name,
                        STATIC_BODY) {
        }

        /**
         * Creates a StaticBody without a collision shape`
         * belonging to the `layer` layers
         */
        explicit StaticBody(const uint32_t layer,
                            const string & name = "StaticBody"):
            PhysicsBody(layer,
                        0,
                        JPH::EActivation::DontActivate,
                        JPH::EMotionType::Static,
                        name,
                        STATIC_BODY) {
        }

        /**
        * Creates a StaticBody without a collision shape`
        */
        explicit StaticBody(const string &name = "StaticBody"):
            PhysicsBody(0,
                        0,
                        JPH::EActivation::DontActivate,
                        JPH::EMotionType::Static,
                        name,
                        STATIC_BODY) {
        }

        ~StaticBody() override = default;
    };

}
