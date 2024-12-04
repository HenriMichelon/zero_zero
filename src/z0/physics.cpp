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
import z0.CollisionObject;
import z0.Signal;
import z0.Tools;

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
                               const JPH::ContactManifold &inManifold) const {
        const auto node1 = reinterpret_cast<CollisionObject*>(body1.GetUserData());
        const auto node2 = reinterpret_cast<CollisionObject*>(body2.GetUserData());
        assert(node1 && node2 && "physics body not associated with a node");
        Application::get().callDeferred([=] {
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
            node1->emit(signal, &event1);
            node2->emit(signal, &event2);
        });
    }

    bool ObjectLayerPairFilterImpl::ShouldCollide(const JPH::ObjectLayer layersAndMask1,
                                                  const JPH::ObjectLayer layersAndMask2) const {
        const auto sourceMask = layersAndMask1 & 0b1111;
        const auto targetLayer = (layersAndMask2 >> 4) & 0b1111;
        // log("ShouldCollide", to_string(sourceMask), to_string(targetLayer));
        return (targetLayer & sourceMask) != 0;
    }

}
