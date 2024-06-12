#pragma once

namespace z0 {

    class ConvexHullShape : public Shape {
    public:
        explicit ConvexHullShape(Node*, const string& resName = "ConvexHullShape");
        explicit ConvexHullShape(const shared_ptr<Node>&, const string& resName = "ConvexHullShape");
    private:
        void tryCreateShape(Node*);
        void createShape(const MeshInstance*);
    };

}