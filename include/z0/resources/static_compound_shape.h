#pragma once

namespace z0 {

    /**
     * Collision shape composed by a collection of SubShape
     */
    class StaticCompoundShape : public Shape {
    public:
        /**
         * Creates a StaticCompoundShape using the `subshaped` collection of Shape
         */
        StaticCompoundShape(const vector<SubShape>& subshapes, const string& resName = "StaticCompoundShape");
    };

}