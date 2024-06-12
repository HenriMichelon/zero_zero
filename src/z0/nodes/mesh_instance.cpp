#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/resources/image.h"
#include "z0/resources/texture.h"
#include "z0/resources/material.h"
#include "z0/resources/mesh.h"
#include "z0/nodes/mesh_instance.h"
#endif

namespace z0 {

    MeshInstance::MeshInstance(const shared_ptr<Mesh>& _mesh, 
                               const string& name): 
        Node{name}, 
        mesh{_mesh}  {
    }

    shared_ptr<Node> MeshInstance::duplicateInstance() {
        return make_shared<MeshInstance>(*this);
    }

}