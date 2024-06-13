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
#endif

namespace z0 {

    CollisionObject::CollisionObject(shared_ptr<Shape>& _shape,
                             uint32_t layer,
                             uint32_t mask,
                             const string& name):
        Node{name},
        collisionLayer{layer},
        collisionMask{mask},
        shape{_shape},
        activationMode{JPH::EActivation::Activate},
        bodyInterface{app()._getBodyInterface()} {
        needPhysics = true;
    }

    void CollisionObject::updateTransform() {
        Node::updateTransform();
        setPositionAndRotation();
    }

    void CollisionObject::updateTransform(const mat4 &parentMatrix) {
        Node::updateTransform(parentMatrix);
        setPositionAndRotation();
    }

    CollisionObject* CollisionObject::_getByBodyId(JPH::BodyID id) {
        return reinterpret_cast<CollisionObject*>(bodyInterface.GetUserData(id));
    }

    bool CollisionObject::shouldCollide(uint32_t layer) const {
        return  (layer & collisionMask) != 0;
    }

    bool CollisionObject::haveCollisionLayer(uint32_t layer) const {
        return collisionLayer & layer;
    }

    bool CollisionObject::haveCollisionMask(uint32_t layer) const {
        return collisionMask & layer;
    }

    void CollisionObject::setCollistionLayer(uint32_t layer, bool value) {
          if (value) {
            collisionLayer |= layer;
        } else {
            collisionLayer &= ~layer;
        }
        bodyInterface.SetObjectLayer(bodyId, collisionLayer << 4 | collisionMask);
    }

    void CollisionObject::setCollistionMask(uint32_t layer, bool value) {
          if (value) {
            collisionMask |= layer;
        } else {
            collisionMask &= ~layer;
        }
        bodyInterface.SetObjectLayer(bodyId, collisionLayer << 4 | collisionMask);
    }

    void CollisionObject::setVelocity(vec3 velocity) {
        // current orientation * velocity
        velocity = toQuat(mat3(localTransform)) * velocity;
        bodyInterface.SetLinearVelocity(bodyId, JPH::Vec3{velocity.x, velocity.y, velocity.z});
    }

    vec3 CollisionObject::getVelocity() const {
        auto velocity = bodyInterface.GetLinearVelocity(bodyId);
        return vec3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
    }

    void CollisionObject::setBodyId(JPH::BodyID id) {
        bodyId = id;
        bodyInterface.SetUserData(bodyId, reinterpret_cast<uint64>(this));
        //log(toString(), " body id ", to_string(id.GetIndexAndSequenceNumber()));
    }

    void CollisionObject::_onPause() {
        bodyInterface.DeactivateBody(bodyId);
    }

    void CollisionObject::_onResume() {
        bodyInterface.ActivateBody(bodyId);
    }

    void CollisionObject::setPositionAndRotation() {
        if (updating || (parent == nullptr)) return;
        auto position = getPositionGlobal();
        auto quat = normalize(toQuat(mat3(worldTransform)));
        bodyInterface.SetPositionAndRotation(
                bodyId,
                JPH::RVec3(position.x, position.y, position.z),
                JPH::Quat(quat.x, quat.y, quat.z, quat.w),
                activationMode);
    }

    void CollisionObject::_onEnterScene() {
        bodyInterface.ActivateBody(bodyId);
        Node::_onEnterScene();
    }

    void CollisionObject::_onExitScene() {
        bodyInterface.DeactivateBody(bodyId);
        Node::_onExitScene();
    }

    void CollisionObject::_physicsUpdate() {
        updating = true;
        JPH::Vec3 position;
        JPH::Quat rotation;
        bodyInterface.GetPositionAndRotation(bodyId, position, rotation);
        auto pos = vec3{position.GetX(), position.GetY(), position.GetZ()};
        if (pos != getPositionGlobal()) {
            //log(z0::toString(pos));
            setPositionGlobal(pos);
        }
        setRotation(quat{rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ()});
        updating = false;
    }

}