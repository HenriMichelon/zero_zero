#pragma once

namespace z0 {

    /**
     * A mesh shape, consisting of triangles, *must* by only used for static geometry. 
     */
    class MeshShape : public Shape {
    public:
        explicit MeshShape(Node*, const string& resName = "MeshShape");
        explicit MeshShape(const shared_ptr<Node>&, const string& resName = "MeshShape");
    private:
        void tryCreateShape(Node*);
        void createShape(const MeshInstance*);
    };

}