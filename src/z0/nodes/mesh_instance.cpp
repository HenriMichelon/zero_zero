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

}
