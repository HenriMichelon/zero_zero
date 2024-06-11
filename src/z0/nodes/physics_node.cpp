#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/resources/image.h"
#include "z0/resources/texture.h"
#include "z0/resources/material.h"
#include "z0/resources/mesh.h"
#include "z0/resources/shape.h"
#include "z0/nodes/collision_node.h"
#include "z0/nodes/physics_node.h"
#include "z0/application.h"
#endif

namespace z0 {

    PhysicsNode::PhysicsNode(shared_ptr<Shape>& _shape,
                             uint32_t layer,
                             uint32_t mask,
                             const string& name):
        CollisionNode{layer, mask, name},
        shape{_shape} {
        activationMode = JPH::EActivation::Activate;
        needPhysics = true;
    }

    void PhysicsNode::updateTransform() {
        Node::updateTransform();
        setPositionAndRotation();
    }

    void PhysicsNode::updateTransform(const mat4 &parentMatrix) {
        Node::updateTransform(parentMatrix);
        setPositionAndRotation();
    }

    void PhysicsNode::setCollistionLayer(uint32_t layer, bool value) {
        CollisionNode::setCollistionLayer(layer, value);
        bodyInterface.SetObjectLayer(bodyId, collisionLayer << 4 | collisionMask);
    }

    void PhysicsNode::setCollistionMask(uint32_t layer, bool value) {
        CollisionNode::setCollistionMask(layer, value);
        bodyInterface.SetObjectLayer(bodyId, collisionLayer << 4 | collisionMask);
    }

    void PhysicsNode::setVelocity(vec3 velocity) {
        // current orientation * velocity
        velocity = toQuat(mat3(localTransform)) * velocity;
        bodyInterface.SetLinearVelocity(bodyId, JPH::Vec3{velocity.x, velocity.y, velocity.z});
    }

    vec3 PhysicsNode::getVelocity() const {
        auto velocity = bodyInterface.GetLinearVelocity(bodyId);
        return vec3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
    }

    void PhysicsNode::setBodyId(JPH::BodyID id) {
        bodyId = id;
        bodyInterface.SetUserData(bodyId, reinterpret_cast<uint64>(this));
        //log(toString(), " body id ", to_string(id.GetIndexAndSequenceNumber()));
    }

    void PhysicsNode::_onPause() {
        bodyInterface.DeactivateBody(bodyId);
    }

    void PhysicsNode::_onResume() {
        bodyInterface.ActivateBody(bodyId);
    }

    void PhysicsNode::setPositionAndRotation() {
        if (updating || (parent == nullptr)) return;
        auto position = getPositionGlobal();
        auto quat = normalize(toQuat(mat3(worldTransform)));
        bodyInterface.SetPositionAndRotation(
                bodyId,
                JPH::RVec3(position.x, position.y, position.z),
                JPH::Quat(quat.x, quat.y, quat.z, quat.w),
                activationMode);
    }

    void PhysicsNode::_onEnterScene() {
        bodyInterface.ActivateBody(bodyId);
        Node::_onEnterScene();
    }

    void PhysicsNode::_onExitScene() {
        bodyInterface.DeactivateBody(bodyId);
        Node::_onExitScene();
    }

    void PhysicsNode::_physicsUpdate() {
        updating = true;
        JPH::Vec3 position;
        JPH::Quat rotation;
        bodyInterface.GetPositionAndRotation(bodyId, position, rotation);
        setPositionGlobal(vec3{position.GetX(), position.GetY(), position.GetZ()});
        setRotation(quat{rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ()});
        updating = false;
    }

}