#pragma once

#include "z0/nodes/node.h"
#include "z0/resources/mesh.h"

namespace z0 {

    class MeshInstance: public Node {
    public:
        explicit MeshInstance(const string& name = "MeshInstance"): Node{name} { }
        explicit MeshInstance(const shared_ptr<Mesh>& _mesh, const string& name = "MeshInstance"): Node{name}, mesh{_mesh} {};

        void setMesh(const shared_ptr<Mesh>& _mesh) { mesh = _mesh; };
        shared_ptr<Mesh> getMesh() const { return mesh; }
        bool isValid() const { return mesh != nullptr; }

    private:
        shared_ptr<Mesh> mesh;
    };

}