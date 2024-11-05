/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.StaticCompoundShape;

import z0.Shape;
import z0.SubShape;

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
