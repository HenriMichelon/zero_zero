/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <cassert>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include "z0/libraries.h"

module z0.Physics;

import z0.Application;
import z0.Constants;
import z0.Signal;
import z0.Tools;

import z0.nodes.CollisionObject;

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

    void ContactListener::emit(const Signal::signal &signal,
                               const JPH::Body &body1,
                               const JPH::Body &body2,
                               const JPH::ContactManifold &inManifold) const {
        const auto node1 = reinterpret_cast<CollisionObject*>(body1.GetUserData());
        const auto node2 = reinterpret_cast<CollisionObject*>(body2.GetUserData());
        //if (signal == "on_collision_starts" && node2->getCollisionMask() == 4) log("emit", node1->getName(), node2->getName());
        assert(node1 && node2 && "physics body not associated with a node");
        const auto pos1 = inManifold.GetWorldSpaceContactPointOn2(0);
        auto event1 = CollisionObject::Collision {
            .position = vec3{pos1.GetX(), pos1.GetY(), pos1.GetZ()},
            .normal = vec3{inManifold.mWorldSpaceNormal.GetX(), inManifold.mWorldSpaceNormal.GetY(), inManifold.mWorldSpaceNormal.GetZ()},
            .object = node2
        };
        const auto pos2 = inManifold.GetWorldSpaceContactPointOn1(0);
        auto event2 = CollisionObject::Collision {
            .position = vec3{pos2.GetX(), pos2.GetY(), pos2.GetZ()},
            .normal = vec3{inManifold.mWorldSpaceNormal.GetX(), inManifold.mWorldSpaceNormal.GetY(), inManifold.mWorldSpaceNormal.GetZ()},
            .object = node1
        };
        node1->_emitDeferred(signal, &event1);
        node2->_emitDeferred(signal, &event2);
    }

    bool ObjectLayerPairFilterImpl::ShouldCollide(const JPH::ObjectLayer layersAndMask1,
                                                  const JPH::ObjectLayer layersAndMask2) const {
        const auto sourceMask = layersAndMask1 & PHYSICS_LAYERS_MASK;
        const auto targetLayer = (layersAndMask2 >> PHYSICS_LAYERS_BITS) & PHYSICS_LAYERS_MASK;
        // if (sourceMask == 4 && targetLayer == 4) {
        // log("ShouldCollide", to_string(sourceMask), to_string(targetLayer), to_string((targetLayer & sourceMask) != 0));
        // }
        return (targetLayer & sourceMask) != 0;
    }

}
