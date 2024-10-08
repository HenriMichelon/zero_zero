module;
#include "z0/jolt.h"
#include "z0/libraries.h"
#include <glm/gtx/quaternion.hpp>

module z0;

import :Constants;
import :Tools;
import :Node;
import :Signal;
import :Shape;
import :Application;

namespace z0 {

    void CollisionObject::setCollistionLayer(const uint32_t layer, const bool value) {
        assert(!bodyId.IsInvalid());
        if (value) {
            collisionLayer |= layer;
        } else {
            collisionLayer &= ~layer;
        }
        bodyInterface.SetObjectLayer(bodyId, collisionLayer << 4 | collisionMask);
    }

    void CollisionObject::setCollistionMask(const uint32_t layer, const bool value) {
        assert(!bodyId.IsInvalid());
        if (value) {
            collisionMask |= layer;
        } else {
            collisionMask &= ~layer;
        }
        bodyInterface.SetObjectLayer(bodyId, collisionLayer << 4 | collisionMask);
    }

    void CollisionObject::setVelocity(const vec3 velocity) {
        assert(!bodyId.IsInvalid());
        if (velocity == VEC3ZERO) {
            bodyInterface.SetLinearVelocity(bodyId, JPH::Vec3::sZero());
        } else {
            // current orientation * velocity
            const auto vel = toQuat(mat3(localTransform)) * velocity;
            bodyInterface.SetLinearVelocity(bodyId, JPH::Vec3{vel.x, vel.y, vel.z});
        }
    }

    vec3 CollisionObject::getVelocity() const {
        assert(!bodyId.IsInvalid());
        const auto velocity = bodyInterface.GetLinearVelocity(bodyId);
        return vec3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
    }

    void CollisionObject::applyForce(const vec3 force) const {
        assert(!bodyId.IsInvalid());
        bodyInterface.AddForce(
                bodyId,
                JPH::Vec3{force.x, force.y, force.z});
    }

    void CollisionObject::applyForce(const vec3 force, const vec3 position) const {
        assert(!bodyId.IsInvalid());
        bodyInterface.AddForce(
                bodyId,
                JPH::Vec3{force.x, force.y, force.z},
                JPH::Vec3{position.x, position.y, position.z});
    }

    bool CollisionObject::wereInContact(const CollisionObject *obj) const {
        assert(!bodyId.IsInvalid());
        return app()._getPhysicsSystem().WereBodiesInContact(bodyId, obj->bodyId);
    }

    void CollisionObject::setProperty(const string &property, const string &value) {
        Node::setProperty(property, value);
        if (property == "layer") {
            setCollistionLayer(stoul(value), true);
        } else if (property == "mask") {
            setCollistionMask(stoul(value), true);
        }
    }

    CollisionObject::CollisionObject(const shared_ptr<Shape> &_shape,
                                     const uint32_t           layer,
                                     const uint32_t           mask,
                                     const string &           name,
                                     const Type               type):
        Node{name, type},
        collisionLayer{layer},
        collisionMask{mask},
        shape{_shape},
        activationMode{JPH::EActivation::Activate},
        bodyInterface{app()._getBodyInterface()} {
    }

    CollisionObject::CollisionObject(const uint32_t layer,
                                     const uint32_t mask,
                                     const string & name,
                                     const Type     type):
        Node{name, type},
        collisionLayer{layer},
        collisionMask{mask},
        shape{nullptr},
        activationMode{JPH::EActivation::Activate},
        bodyInterface{app()._getBodyInterface()} {
    }

    void CollisionObject::setPositionAndRotation() {
        if (updating || bodyId.IsInvalid())
            return;
        const auto position = getPositionGlobal();
        const auto quat     = normalize(toQuat(mat3(worldTransform)));
        bodyInterface.SetPositionAndRotation(
                bodyId,
                JPH::RVec3(position.x, position.y, position.z),
                JPH::Quat(quat.x, quat.y, quat.z, quat.w),
                activationMode);
    }

    void CollisionObject::setBodyId(const JPH::BodyID id) {
        bodyId = id;
        bodyInterface.SetUserData(bodyId, reinterpret_cast<uint64>(this));
        //log(toString(), " body id ", to_string(id.GetIndexAndSequenceNumber()));
    }

    CollisionObject *CollisionObject::_getByBodyId(const JPH::BodyID id) const {
        assert(!bodyId.IsInvalid());
        return reinterpret_cast<CollisionObject *>(bodyInterface.GetUserData(id));
    }

    void CollisionObject::_updateTransform() {
        Node::_updateTransform();
        setPositionAndRotation();
    }

    void CollisionObject::_updateTransform(const mat4 &parentMatrix) {
        Node::_updateTransform(parentMatrix);
        setPositionAndRotation();
    }

    void CollisionObject::_physicsUpdate(const float delta) {
        assert(!bodyId.IsInvalid());
        Node::_physicsUpdate(delta);
        updating = true;
        JPH::Vec3 position;
        JPH::Quat rotation;
        bodyInterface.GetPositionAndRotation(bodyId, position, rotation);
        const auto pos = vec3{position.GetX(), position.GetY(), position.GetZ()};
        if (pos != getPositionGlobal()) {
            setPositionGlobal(pos);
        }
        setRotation(quat{rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ()});
        updating = false;
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

    void CollisionObject::_onPause() {
        if (!bodyId.IsInvalid()) {
            bodyInterface.DeactivateBody(bodyId);
        }
    }

    void CollisionObject::_onResume() {
        if (!bodyId.IsInvalid()) {
            bodyInterface.ActivateBody(bodyId);
        }
    }

    const Signal::signal CollisionObject::on_collision_starts   = "on_collision_starts";
    const Signal::signal CollisionObject::on_collision_persists = "on_collision_persists";

}
