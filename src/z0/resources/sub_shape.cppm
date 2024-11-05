/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.SubShape;

import z0.Shape;

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
