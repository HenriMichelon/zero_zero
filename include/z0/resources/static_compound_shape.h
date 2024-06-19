#pragma once

namespace z0 {

    /**
     * Collision shape composed by a collection of SubShape
     */
    class StaticCompoundShape : public Shape {
    public:
        StaticCompoundShape(const vector<SubShape>& subshapes, const string& resName = "StaticCompoundShape");
    };

}