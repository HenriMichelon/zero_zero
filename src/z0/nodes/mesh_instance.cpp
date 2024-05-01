#include "z0/nodes/mesh_instance.h"

namespace z0 {

    shared_ptr<Node> MeshInstance::duplicateInstance() {
        return make_shared<MeshInstance>(*this);
    }

}