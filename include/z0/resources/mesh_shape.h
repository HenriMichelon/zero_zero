#pragma once

namespace z0 {

    /**
     * A mesh shape, consisting of triangles, *must* by only used with a StaticBody (like a terrain for example)
     */
    class MeshShape : public Shape {
    public:
        /**
         * Creates a MeshShape using the triangles of the Mesh of first MeshInstance found in the `node` tree
         */
        explicit MeshShape(Node*, const string& resName = "MeshShape");

        /**
         * Creates a MeshShape using the triangles of the Mesh of first MeshInstance found in the `node` tree
         */
        explicit MeshShape(const shared_ptr<Node>&, const string& resName = "MeshShape");
    private:
        void tryCreateShape(Node*);
        void createShape(const MeshInstance*);
    };

}