/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/jolt.h"
#include "z0/libraries.h"

export module z0.ConvexHullShape;

import z0.Tools;
import z0.Node;
import z0.Mesh;
import z0.MeshInstance;
import z0.Shape;

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

        /**
         * Creates a ConvexHullShape using a list of vertices
         */
        ConvexHullShape(const vector<vec3>& points, const string &resName);

        shared_ptr<Resource> duplicate()  const override;

    private:
        vector<vec3> points;

        void tryCreateShape(const shared_ptr<Node> &node);

        void createShape(const shared_ptr<MeshInstance>& meshInstance);

        void createShape(const shared_ptr<Mesh> &mesh);

        void createShape();
    };

}
