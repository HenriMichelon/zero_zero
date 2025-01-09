/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Character/Character.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <cassert>
#include "z0/libraries.h"

module z0.Physics;

import z0.Application;
import z0.Constants;
import z0.Signal;
import z0.Tools;

import z0.nodes.CollisionObject;
import z0.nodes.Node;

namespace z0 {

    JPH::ValidateResult	ContactListener::OnContactValidate(const JPH::Body &inBody1,
                                      const JPH::Body &inBody2,
                                      JPH::RVec3Arg inBaseOffset,
                                      const JPH::CollideShapeResult &inCollisionResult) {
        const auto node1 = reinterpret_cast<CollisionObject*>(inBody1.GetUserData());
        const auto node2 = reinterpret_cast<CollisionObject*>(inBody2.GetUserData());
        assert(node1 && node2 && "physics body not associated with a node");
        return (node1->isProcessed() && node2->isProcessed())  ?
            JPH::ValidateResult::AcceptAllContactsForThisBodyPair :
            JPH::ValidateResult::RejectAllContactsForThisBodyPair;
    }

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
        assert(node1 && node2 && "physics body not associated with a node");
        const auto normal = vec3{inManifold.mWorldSpaceNormal.GetX(), inManifold.mWorldSpaceNormal.GetY(), inManifold.mWorldSpaceNormal.GetZ()};
        if (node1->getType() != Node::CHARACTER) {
            const auto pos1 = inManifold.GetWorldSpaceContactPointOn2(0);
            auto event1 = CollisionObject::Collision {
                .position = vec3{pos1.GetX(), pos1.GetY(), pos1.GetZ()},
                .normal = normal,
                .object = node2
            };
            // log("ContactListener::emit 1", signal, node1->getName(), node2->getName());
            app().callDeferred([this, signal, node1, event1]{
               node1->emit(signal, (void*)&event1);
            });
        }
        const auto pos2 = inManifold.GetWorldSpaceContactPointOn1(0);
        auto event2 = CollisionObject::Collision {
            .position = vec3{pos2.GetX(), pos2.GetY(), pos2.GetZ()},
            .normal = normal,
            .object = node1
        };
        // log("ContactListener::emit 2", signal, node1->getName(), node2->getName());
        app().callDeferred([this, signal, node2, event2]{
           node2->emit(signal, (void*)&event2);
        });
    }

    bool ObjectLayerPairFilterImpl::ShouldCollide(const JPH::ObjectLayer inObject1,
                                                  const JPH::ObjectLayer inObject2) const {
        // if (inObject2 != 1)
            // log("ShouldCollide", to_string(inObject1), to_string(inObject2),
                // to_string(JPH::ObjectLayerPairFilterTable::ShouldCollide(inObject1, inObject2)));
        // return true;
        return JPH::ObjectLayerPairFilterTable::ShouldCollide(inObject1, inObject2);
    }

}
