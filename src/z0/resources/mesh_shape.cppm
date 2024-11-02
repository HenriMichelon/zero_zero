/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/jolt.h"
#include "z0/libraries.h"

export module z0:MeshShape;

import :Node;
import :MeshInstance;
import :Shape;

export namespace z0 {

    /**
     * A mesh shape, consisting of triangles. *Must* only be used with a StaticBody (like a terrain for example)
     */
    class MeshShape : public Shape {
    public:
        /**
         * Creates a MeshShape using the triangles of the Mesh of first MeshInstance found in the `node` tree
         */
        explicit MeshShape(const shared_ptr<Node> &node, const string &resName = "MeshShape");

    private:
        void tryCreateShape(const shared_ptr<Node>& node);

        void createShape(const shared_ptr<MeshInstance>& meshInstance);
    };

}
