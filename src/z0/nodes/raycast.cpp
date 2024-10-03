module;
#include "z0/jolt.h"
#include "z0/libraries.h"
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>

module z0;

import :RayCast;

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
        const auto          position    = getPositionGlobal();
        const auto          worldTarget = toGlobal(target);
        const JPH::RRayCast ray{
                JPH::Vec3{position.x, position.y, position.z},
                JPH::Vec3{worldTarget.x, worldTarget.y, worldTarget.z}
        };
        JPH::RayCastResult result;
        if (app()._getPhysicsSystem().GetNarrowPhaseQuery().CastRay(
                ray,
                result,
                broadPhaseLayerFilter,
                *this,
                *this)) {
            collider      = reinterpret_cast<CollisionObject *>(app()._getBodyInterface().GetUserData(result.mBodyID));
            auto posInRay = ray.GetPointOnRay(result.mFraction);
            hitPoint      = vec3{posInRay.GetX(), posInRay.GetY(), posInRay.GetZ()};
        } else {
            collider = nullptr;
        }
    }

    bool RayCast::ShouldCollide(const JPH::ObjectLayer inLayer) const {
        const auto targetLayer = (inLayer >> 4) & 0b1111;
        return (targetLayer & collisionMask) != 0;
    }

    bool RayCast::ShouldCollideLocked(const JPH::Body &inBody) const {
        const auto *node = reinterpret_cast<CollisionObject *>(inBody.GetUserData());
        return (node != nullptr) && (!(excludeParent && (node == parent)));
    }

}
