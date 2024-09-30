module;
#include "z0/libraries.h"

export module Z0:SubShape;

import :Shape;

export namespace z0 {

    /**
     * Sub shape composing a StaticCompoundShape
     */
    struct SubShape {
        /**
         * The geometry shape
         */
        shared_ptr<Shape> shape;
        /**
         * Local space position
         */
        vec3 position{0.0f};
        /**
         * Local space rotation
         */
        vec3 rotation{0.0f};
    };


}
