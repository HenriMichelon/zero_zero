/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

module z0.nodes.MeshInstance;

import z0.nodes.Node;

import z0.resources.Mesh;

namespace z0 {

    MeshInstance::MeshInstance(const shared_ptr<Mesh> &mesh, const string &name):
        Node{name, MESH_INSTANCE},
        mesh{mesh} {
    }

    // MeshInstance::MeshInstance(const MeshInstance & original):
    //     Node{original.name, MESH_INSTANCE},
    //     mesh{original.mesh},
    //     worldAABB{original.worldAABB},
    //     outlined{original.outlined},
    //     outlineMaterial{original.outlineMaterial} {
    //     log("mesh instance copy");
    // }

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
    }

    void MeshInstance::setCastShadows(const bool castShadows) {
        this->castShadows = castShadows;
        Node::setCastShadows(castShadows);
    }


}
