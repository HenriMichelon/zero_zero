module;
#include "z0/libraries.h"

export module z0:StaticCompoundShape;

import :Shape;
import :SubShape;

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
