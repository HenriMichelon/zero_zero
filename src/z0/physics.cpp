#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/resources/image.h"
#include "z0/resources/texture.h"
#include "z0/resources/material.h"
#include "z0/resources/shape.h"
#include "z0/resources/mesh.h"
#include "z0/nodes/collision_object.h"
#endif

namespace z0 {
    void ContactListener::OnContactAdded(const JPH::Body &inBody1,
                                         const JPH::Body &inBody2,
                                         const JPH::ContactManifold &inManifold,
                                         JPH::ContactSettings &ioSettings) {
        auto node1 = reinterpret_cast<CollisionObject*>(inBody1.GetUserData());
        auto node2 = reinterpret_cast<CollisionObject*>(inBody2.GetUserData());
        assert(node1 && node2 && "physics body not associated with a node");
        auto pos = inManifold.GetWorldSpaceContactPointOn2(0);
        node1->onCollisionStarts({
            vec3{pos.GetX(), pos.GetY(), pos.GetZ()},
            vec3{inManifold.mWorldSpaceNormal.GetX(), inManifold.mWorldSpaceNormal.GetY(), inManifold.mWorldSpaceNormal.GetZ()},
            node2
        });
    }

    bool ObjectLayerPairFilterImpl::ShouldCollide(JPH::ObjectLayer layersAndMask1,
                                                  JPH::ObjectLayer layersAndMask2) const {
        auto sourceMask = layersAndMask1 & 0b1111;
        auto targetLayer = (layersAndMask2 >> 4) & 0b1111;
        return (targetLayer & sourceMask) != 0;
    }

}
