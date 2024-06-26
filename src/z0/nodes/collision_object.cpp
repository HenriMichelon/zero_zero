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

    const Signal::signal CollisionObject::on_collision_starts = "on_collision_starts";
    const Signal::signal CollisionObject::on_collision_persists = "on_collision_persists";

    CollisionObject::CollisionObject(uint32_t layer,
                                    uint32_t mask,
                                    const string& name):
        Node{name},
        collisionLayer{layer},
        collisionMask{mask},
        shape{nullptr},
        activationMode{JPH::EActivation::Activate},
        bodyInterface{app()._getBodyInterface()} {
    }

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
    }

    void CollisionObject::_updateTransform() {
        Node::_updateTransform();
        setPositionAndRotation();
    }

    void CollisionObject::_updateTransform(const mat4 &parentMatrix) {
        Node::_updateTransform(parentMatrix);
        setPositionAndRotation();
    }

    CollisionObject* CollisionObject::_getByBodyId(JPH::BodyID id) {
        assert(!bodyId.IsInvalid());
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
        assert(!bodyId.IsInvalid());
          if (value) {
            collisionLayer |= layer;
        } else {
            collisionLayer &= ~layer;
        }
        bodyInterface.SetObjectLayer(bodyId, collisionLayer << 4 | collisionMask);
    }

    void CollisionObject::setCollistionMask(uint32_t layer, bool value) {
        assert(!bodyId.IsInvalid());
          if (value) {
            collisionMask |= layer;
        } else {
            collisionMask &= ~layer;
        }
        bodyInterface.SetObjectLayer(bodyId, collisionLayer << 4 | collisionMask);
    }

    void CollisionObject::setVelocity(vec3 velocity) {
        assert(!bodyId.IsInvalid());
        if (velocity == VEC3ZERO) {
            bodyInterface.SetLinearVelocity(bodyId, JPH::Vec3::sZero());
        } else {
            // current orientation * velocity
            velocity = toQuat(mat3(localTransform)) * velocity;
            bodyInterface.SetLinearVelocity(bodyId, JPH::Vec3{velocity.x, velocity.y, velocity.z});
        }
    }

    vec3 CollisionObject::getVelocity() const {
        assert(!bodyId.IsInvalid());
        auto velocity = bodyInterface.GetLinearVelocity(bodyId);
        return vec3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
    }

    void CollisionObject::setBodyId(JPH::BodyID id) {
        bodyId = id;
        bodyInterface.SetUserData(bodyId, reinterpret_cast<uint64>(this));
        //log(toString(), " body id ", to_string(id.GetIndexAndSequenceNumber()));
    }

    void CollisionObject::_onPause() {
        assert(!bodyId.IsInvalid());
        bodyInterface.DeactivateBody(bodyId);
    }

    void CollisionObject::_onResume() {
        assert(!bodyId.IsInvalid());
        bodyInterface.ActivateBody(bodyId);
    }

    void CollisionObject::setPositionAndRotation() {
        if (updating || bodyId.IsInvalid()) return;
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
        setPositionAndRotation();
        Node::_onEnterScene();
    }

    void CollisionObject::_onExitScene() {
        bodyInterface.DeactivateBody(bodyId);
        Node::_onExitScene();
    }

    void CollisionObject::_physicsUpdate(float delta) {
        assert(!bodyId.IsInvalid());
        Node::_physicsUpdate(delta);
        updating = true;
        JPH::Vec3 position;
        JPH::Quat rotation;
        bodyInterface.GetPositionAndRotation(bodyId, position, rotation);
        auto pos = vec3{position.GetX(), position.GetY(), position.GetZ()};
        if (pos != getPositionGlobal()) {
            setPositionGlobal(pos);
        }
        setRotation(quat{rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ()});
        updating = false;
    }

    void CollisionObject::applyForce(vec3 force) {
        assert(!bodyId.IsInvalid());
        bodyInterface.AddForce(
            bodyId, 
            JPH::Vec3{force.x, force.y, force.z});
    }

    void CollisionObject::applyForce(vec3 force, vec3 position) {
        assert(!bodyId.IsInvalid());
        bodyInterface.AddForce(
            bodyId, 
            JPH::Vec3{force.x, force.y, force.z}, 
            JPH::Vec3{position.x, position.y, position.z});
    }

    bool CollisionObject::wereInContact(CollisionObject* obj) {
        assert(!bodyId.IsInvalid());
        return app()._getPhysicsSystem().WereBodiesInContact(bodyId, obj->bodyId);
    }

}