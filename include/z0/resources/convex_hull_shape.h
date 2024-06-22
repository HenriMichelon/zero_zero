#pragma once

namespace z0 {

    /**
     * A convex hull collision shape
     */
    class ConvexHullShape : public Shape {
    public:
        /**
         * Creates a ConvexHullShape using the vertices of the Mesh of first MeshInstance found in the `node` tree
         */
        explicit ConvexHullShape(Node*node, const string& resName = "ConvexHullShape");

        /**
         * Creates a ConvexHullShape using the vertices of the Mesh of the first MeshInstance found in the `node` tree
         */
        explicit ConvexHullShape(const shared_ptr<Node>&node, const string& resName = "ConvexHullShape");

    private:
        void tryCreateShape(Node*);
        void createShape(const MeshInstance*);
    };

}