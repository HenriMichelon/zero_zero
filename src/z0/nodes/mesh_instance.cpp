module;
#include "z0/libraries.h"

module z0;

import :MeshInstance;

namespace z0 {

    MeshInstance::MeshInstance(const shared_ptr<Mesh> &mesh, const string &name):
        Node{name, MESH_INSTANCE},
        mesh{mesh} {
    }

    shared_ptr<Node> MeshInstance::duplicateInstance() {
        return make_shared<MeshInstance>(*this);
    }
    
            
    void MeshInstance::_updateTransform(const mat4 &parentMatrix) {
        Node::_updateTransform(parentMatrix);
        worldAABB = mesh->getAABB().toGlobal(worldTransform) ; 
    }

    void MeshInstance::_updateTransform() {
        Node::_updateTransform() ;
        worldAABB = mesh->getAABB().toGlobal(worldTransform) ;
    };


}
