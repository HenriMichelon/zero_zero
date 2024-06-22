#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/resources/image.h"
#include "z0/resources/texture.h"
#include "z0/resources/material.h"
#include "z0/resources/shape.h"
#include "z0/resources/mesh.h"
#include "z0/nodes/collision_object.h"
#include "z0/application.h"
#endif

namespace z0 {

    void ContactListener::OnContactAdded(const JPH::Body &inBody1,
                                         const JPH::Body &inBody2,
                                         const JPH::ContactManifold &inManifold,
                                         JPH::ContactSettings &ioSettings) {
        emit(CollisionObject::on_collision_starts, inBody1, inBody2, inManifold);
    }

    void ContactListener::OnContactPersisted(const JPH::Body &inBody1, 
                        const JPH::Body &inBody2, 
                        const JPH::ContactManifold &inManifold, 
                        JPH::ContactSettings &ioSettings) {
        emit(CollisionObject::on_collision_persists, inBody1, inBody2, inManifold);
    }

    void ContactListener::emit(Signal::signal signal,
                               const JPH::Body &body1, 
                               const JPH::Body &body2, 
                               const JPH::ContactManifold &inManifold) {
        auto node1 = reinterpret_cast<CollisionObject*>(body1.GetUserData());
        auto node2 = reinterpret_cast<CollisionObject*>(body2.GetUserData());
        assert(node1 && node2 && "physics body not associated with a node");
        auto pos = inManifold.GetWorldSpaceContactPointOn2(0);
        auto event = CollisionObject::Collision {
            .position = vec3{pos.GetX(), pos.GetY(), pos.GetZ()},
            .normal = vec3{inManifold.mWorldSpaceNormal.GetX(), inManifold.mWorldSpaceNormal.GetY(), inManifold.mWorldSpaceNormal.GetZ()},
            .object = node2
        };
        node1->emit(signal, &event);
    }

    bool ObjectLayerPairFilterImpl::ShouldCollide(JPH::ObjectLayer layersAndMask1,
                                                  JPH::ObjectLayer layersAndMask2) const {
        auto sourceMask = layersAndMask1 & 0b1111;
        auto targetLayer = (layersAndMask2 >> 4) & 0b1111;
        return (targetLayer & sourceMask) != 0;
    }

}
