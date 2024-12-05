/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.resources.StaticCompoundShape;

import z0.resources.Shape;
import z0.resources.SubShape;

export namespace z0 {

    /**
     * Collision shape composed by a collection of SubShape
     */
    class StaticCompoundShape : public Shape {
    public:
        /**
         * Creates a StaticCompoundShape using the `subshaped` collection of Shape
         */
        explicit StaticCompoundShape(const vector<SubShape> &subshapes, const string &resName = "StaticCompoundShape");
    };

}
