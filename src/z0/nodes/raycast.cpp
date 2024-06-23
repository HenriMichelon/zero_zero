#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/resources/image.h"
#include "z0/resources/texture.h"
#include "z0/resources/material.h"
#include "z0/resources/mesh.h"
#include "z0/resources/shape.h"
#include "z0/nodes/collision_object.h"
#include "z0/application.h"
#include "z0/nodes/raycast.h"
#endif

namespace z0 {

    RayCast::RayCast(vec3 _target, 
                     uint32_t mask,
                     const string& name): 
        Node(name), 
        target{_target},
        collisionMask{mask} {
    }

    bool RayCast::isColliding() const {
        return collider != nullptr;
    }

    CollisionObject* RayCast::getCollider() const {
        return collider;
    }

    vec3 RayCast::getCollisionPoint() const {
        return hitPoint;
    }

    bool RayCast::ShouldCollide (JPH::ObjectLayer inLayer) const {
        auto targetLayer = (inLayer >> 4) & 0b1111;
        return (targetLayer & collisionMask) != 0;
    }

    bool RayCast::ShouldCollideLocked (const JPH::Body &inBody) const {
        auto* node = reinterpret_cast<CollisionObject*>(inBody.GetUserData());
        return (node != nullptr) && (!(excludeParent && (node == parent)));
    }

    void RayCast::setExcludeParent(bool exclude) {
        excludeParent = exclude;
    }

    void RayCast::setTarget(vec3 t) {
        target = t;
    }

    void RayCast::forceRaycastUpdate() {
        _physicsUpdate(0.0f);
    }

    void RayCast::_physicsUpdate(float delta) {
        Node::_physicsUpdate(delta);
        const auto position = getPositionGlobal();
        const auto worldTarget = toGlobal(target);
        const JPH::RRayCast ray{
            JPH::Vec3{position.x, position.y, position.z},
            JPH::Vec3{worldTarget.x, worldTarget.y, worldTarget.z}
        };
        JPH::RayCastResult result;
        if (app()._getPhysicsSystem().GetNarrowPhaseQuery().CastRay(ray, result, broadPhaseLayerFilter, *this, *this)) {
            collider = reinterpret_cast<CollisionObject*>(app()._getBodyInterface().GetUserData(result.mBodyID));
            auto posInRay = ray.GetPointOnRay(result.mFraction);
            hitPoint = vec3{posInRay.GetX(), posInRay.GetY(), posInRay.GetZ()};
        } else {
            collider = nullptr;
        }
    }

}
