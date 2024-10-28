module;
#include "z0/jolt.h"
#include "z0/libraries.h"

export module z0:ConvexHullShape;

import :Node;
import :Mesh;
import :MeshInstance;
import :Shape;

export namespace z0 {

    /**
     * A convex hull collision shape
     */
    class ConvexHullShape : public Shape {
    public:
        /**
         * Creates a ConvexHullShape using the vertices of the Mesh of the first MeshInstance found in the `node` tree.
         * Uses the local transform of the node when creating the shape.
         */
        explicit ConvexHullShape(const shared_ptr<Node> &node, const string &resName = "ConvexHullShape");

        /**
         * Creates a ConvexHullShape using the vertices of the Mesh
         */
        explicit ConvexHullShape(const shared_ptr<Mesh> &mesh, const string &resName = "ConvexHullShape");

    private:
        void tryCreateShape(const shared_ptr<Node> &node);

        void createShape(const MeshInstance *meshInstance);

        void createShape(const shared_ptr<Mesh> &mesh);
    };

}
