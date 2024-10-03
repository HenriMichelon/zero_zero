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
                            uint32_t                 layer,
                            const string &           name = "StaticBody");

        /**
         * Creates a StaticBody without a collision shape`
         * belonging to the `layer` layers
         */
        explicit StaticBody(uint32_t      layer,
                            const string &name = "StaticBody");

        /**
        * Creates a StaticBody without a collision shape`
        */
        explicit StaticBody(const string &name = "StaticBody");

        ~StaticBody() override = default;
    };

}
