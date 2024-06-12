#pragma once

namespace z0 {

    class StaticCompoundShape : public Shape {
    public:
        StaticCompoundShape(const vector<SubShape>& subshapes, const string& resName = "StaticCompoundShape");
    };

}