#include "z0/base.h"
#include "z0/resources/shape.h"
#include "z0/nodes/physics_node.h"
#include "z0/nodes/physics_body.h"

namespace z0 {

    PhysicsBody::PhysicsBody(shared_ptr<Shape>& _shape,
                             uint32_t layer,
                             uint32_t mask,
                             JPH::EActivation _activationMode,
                             JPH::EMotionType _motionType,
                             const string& name):
            PhysicsNode{_shape, layer, mask, name},
        motionType{_motionType} {
        activationMode = _activationMode;
        auto position = getPositionGlobal();
        auto quat = normalize(toQuat(mat3(worldTransform)));
        const JPH::BodyCreationSettings settings{
                shape->_getShape(),
                JPH::RVec3(position.x, position.y, position.z),
                JPH::Quat(quat.x, quat.y, quat.z, quat.w),
                motionType,
                collisionLayer << 4 | collisionMask
        };
        setBodyId(bodyInterface.CreateAndAddBody(settings, JPH::EActivation::DontActivate));
    }

    PhysicsBody::~PhysicsBody() {
        bodyInterface.RemoveBody(_getBodyId());
        bodyInterface.DestroyBody(_getBodyId());
    }

    void PhysicsBody::setGravityScale(float value) {
        bodyInterface.SetGravityFactor(_getBodyId(), value);
    }

}