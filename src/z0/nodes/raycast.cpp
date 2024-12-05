/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include "z0/libraries.h"

module z0.RayCast;

import z0.Application;
import z0.CollisionObject;
import z0.Constants;

namespace z0 {

    RayCast::RayCast(const vec3 &target, const uint32_t mask, const string &name):
        Node{name, RAYCAST},
        target{target},
        collisionMask{mask} {
    }

    RayCast::RayCast(const string &name):
        Node{name, RAYCAST} {
    }

    void RayCast::_physicsUpdate(const float delta) {
        Node::_physicsUpdate(delta);
        const auto position = getPositionGlobal();
        const auto worldDirection = toGlobal(target) - position;
        const JPH::RRayCast ray{
            JPH::Vec3{position.x, position.y, position.z },
            JPH::Vec3{worldDirection.x, worldDirection.y, worldDirection.z}
        };
        JPH::RayCastResult result;
        if (Application::get()._getPhysicsSystem().GetNarrowPhaseQuery().CastRay(
                ray,
                result,
                broadPhaseLayerFilter,
                *this,
                *this)) {
            collider = reinterpret_cast<CollisionObject *>(
                    Application::get()._getBodyInterface().GetUserData(result.mBodyID));
            const auto posInRay = ray.GetPointOnRay(result.mFraction);
            hitPoint = vec3{posInRay.GetX(), posInRay.GetY(), posInRay.GetZ()};
        } else {
            collider = nullptr;
        }
    }

    bool RayCast::ShouldCollide(const JPH::ObjectLayer inLayer) const {
        const auto targetLayer = (inLayer >> PHYSICS_LAYERS_BITS) & PHYSICS_LAYERS_MASK;
        return (targetLayer & collisionMask) != 0;
    }

    bool RayCast::ShouldCollideLocked(const JPH::Body &inBody) const {
        const auto *node = reinterpret_cast<CollisionObject *>(inBody.GetUserData());
        return (node != nullptr) && (!(excludeParent && (node == parent)));
    }

}
