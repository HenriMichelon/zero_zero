#include "z0/base.h"
#include "z0/resources/shape.h"
#include "z0/nodes/physics_node.h"

namespace z0 {
    void ContactListener::OnContactAdded(const JPH::Body &inBody1,
                                         const JPH::Body &inBody2,
                                         const JPH::ContactManifold &inManifold,
                                         JPH::ContactSettings &ioSettings) {
        auto node1 = reinterpret_cast<PhysicsNode*>(inBody1.GetUserData());
        auto node2 = reinterpret_cast<PhysicsNode*>(inBody2.GetUserData());
        node1->onCollisionStarts(node2);
    }

    bool ObjectLayerPairFilterImpl::ShouldCollide(JPH::ObjectLayer layersAndMask1,
                                                  JPH::ObjectLayer layersAndMask2) const {
        auto sourceMask = layersAndMask1 & 0b1111;
        auto targetLayer = (layersAndMask2 >> 4) & 0b1111;
        return (targetLayer & sourceMask) != 0;
    }

}
