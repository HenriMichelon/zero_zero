#include "z0/nodes/physics_body.h"
#include "z0/application.h"

#include <glm/gtx/quaternion.hpp>

#include <Jolt/Physics/Body/BodyCreationSettings.h>

namespace z0 {

    PhysicsBody::PhysicsBody(shared_ptr<Shape>& _shape,
                             uint32_t layer,
                             uint32_t mask,
                             JPH::EActivation _activationMode,
                             JPH::EMotionType _motionType,
                             const string& name):
        Node{name},
        bodyInterface{Application::get()._getBodyInterface()},
        shape{_shape},
        activationMode{_activationMode},
        motionType{_motionType},
        collisionLayer{layer},
        collisionMask{mask} {
        const JPH::BodyCreationSettings settings{
                shape->_getShape(),
                JPH::RVec3(0.0f, 0.0f, 0.0f),
                JPH::Quat::sIdentity(),
                motionType,
                collisionLayer << 4 | collisionMask
        };
        bodyId = bodyInterface.CreateAndAddBody(settings, JPH::EActivation::DontActivate);
        setPositionAndRotation();
        needPhysics = true;
    }

    PhysicsBody::~PhysicsBody() {
        //bodyInterface.DestroyBody(bodyId);
    }

    void PhysicsBody::_onEnterScene() {
        Node::_onEnterScene();
        bodyInterface.ActivateBody(bodyId);
    }

    void PhysicsBody::_onExitScene() {
        bodyInterface.DeactivateBody(bodyId);
        Node::_onExitScene();
    }

    void PhysicsBody::_physicsUpdate() {
        updating = true;
        JPH::Vec3 position;
        JPH::Quat rotation;
        bodyInterface.GetPositionAndRotation(bodyId, position, rotation);
        setPositionGlobal(vec3{position.GetX(), position.GetY(), position.GetZ()});
        setRotation(quat{rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ()});
        updating = false;
    }

    void PhysicsBody::setPositionAndRotation() {
        if (updating || (parent == nullptr)) return;
        auto position = getPositionGlobal();
        auto quat = toQuat(mat3(worldTransform));
        bodyInterface.SetPositionAndRotation(
                bodyId,
                JPH::RVec3(position.x, position.y, position.z),
                JPH::Quat(quat.x, quat.y, quat.z, quat.w),
                activationMode);
    }

    void PhysicsBody::updateTransform() {
        Node::updateTransform();
        setPositionAndRotation();
    }

    void PhysicsBody::updateTransform(const mat4 &parentMatrix) {
        Node::updateTransform(parentMatrix);
        setPositionAndRotation();
    }

    bool PhysicsBody::haveCollisionLayer(uint32_t layer) const {
        return collisionLayer & layer;
    }

    bool PhysicsBody::haveCollisionMask(uint32_t layer) const {
        return collisionMask & layer;
    }

    void PhysicsBody::setCollistionLayer(uint32_t layer, bool value) {
        if (value) {
            collisionLayer |= layer;
        } else {
            collisionLayer &= ~layer;
        }
        bodyInterface.SetObjectLayer(bodyId, collisionLayer << 4 | collisionMask);
    }

    void PhysicsBody::setCollistionMask(uint32_t layer, bool value) {
        if (value) {
            collisionMask |= layer;
        } else {
            collisionMask &= ~layer;
        }
        bodyInterface.SetObjectLayer(bodyId, collisionLayer << 4 | collisionMask);
    }

}