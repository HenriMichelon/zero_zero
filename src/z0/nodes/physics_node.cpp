#include "z0/nodes/physics_node.h"
#include "z0/application.h"

#include <glm/gtx/quaternion.hpp>

namespace z0 {

    PhysicsNode::PhysicsNode(shared_ptr<Shape>& _shape,
                             uint32_t layer,
                             uint32_t mask,
                             const string& name):
        Node{name},
        collisionLayer{layer},
        collisionMask{mask},
        shape{_shape},
        bodyInterface{Application::get()._getBodyInterface()} {
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

    bool PhysicsNode::haveCollisionLayer(uint32_t layer) const {
        return collisionLayer & layer;
    }

    bool PhysicsNode::haveCollisionMask(uint32_t layer) const {
        return collisionMask & layer;
    }

    void PhysicsNode::setCollistionLayer(uint32_t layer, bool value) {
        if (value) {
            collisionLayer |= layer;
        } else {
            collisionLayer &= ~layer;
        }
        bodyInterface.SetObjectLayer(bodyId, collisionLayer << 4 | collisionMask);
    }

    void PhysicsNode::setCollistionMask(uint32_t layer, bool value) {
        if (value) {
            collisionMask |= layer;
        } else {
            collisionMask &= ~layer;
        }
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
        Node::_onEnterScene();
        bodyInterface.ActivateBody(bodyId);
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