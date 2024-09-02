module;
#include "z0/jolt.h"
#include "z0/modules.h"
#include <glm/gtx/quaternion.hpp>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

export module Z0:PhysicsBody;

import :CollisionObject;
import :Shape;

export namespace z0 {

    /**
     * Base class for 3D game objects affected by physics.
     */
    class PhysicsBody: public CollisionObject {
    public:
        ~PhysicsBody() override;

        /**
         * Sets an artificial gravity factor
         */
        void setGravityScale(float value);

    protected:
        PhysicsBody(shared_ptr<Shape>& shape,
                    uint32_t layer,
                    uint32_t mask,
                    JPH::EActivation activationMode,
                    JPH::EMotionType motionType,
                    const string& name);

        PhysicsBody(uint32_t layer,
                    uint32_t mask,
                    JPH::EActivation activationMode,
                    JPH::EMotionType motionType,
                    const string& name);

        void setShape(shared_ptr<Shape> shape);

    private:
        JPH::EMotionType motionType;
    };


    PhysicsBody::PhysicsBody(shared_ptr<Shape>& _shape,
                             uint32_t layer,
                             uint32_t mask,
                             JPH::EActivation _activationMode,
                             JPH::EMotionType _motionType,
                             const string& name):
            CollisionObject{_shape, layer, mask, name},
        motionType{_motionType} {
        activationMode = _activationMode;
        setShape(shape);
    }

    PhysicsBody::PhysicsBody(uint32_t layer,
                             uint32_t mask,
                             JPH::EActivation _activationMode,
                             JPH::EMotionType _motionType,
                             const string& name):
            CollisionObject{layer, mask, name},
        motionType{_motionType} {
        activationMode = _activationMode;
    }

    void PhysicsBody::setShape(shared_ptr<Shape> shape) {
        auto position = getPositionGlobal();
        auto quat = normalize(toQuat(mat3(worldTransform)));
        shape->setAttachedToNode();
        const JPH::BodyCreationSettings settings{
            shape->_getShapeSettings(),
            JPH::RVec3{position.x, position.y, position.z},
            JPH::Quat{quat.x, quat.y, quat.z, quat.w},
            motionType,
            collisionLayer << 4 | collisionMask
    };
        setBodyId(bodyInterface.CreateAndAddBody(settings, JPH::EActivation::DontActivate));
    }

    PhysicsBody::~PhysicsBody() {
        if (!_getBodyId().IsInvalid()) {
            bodyInterface.RemoveBody(_getBodyId());
            bodyInterface.DestroyBody(_getBodyId());
        }
    }

    void PhysicsBody::setGravityScale(float value) {
        assert(!_getBodyId().IsInvalid());
        bodyInterface.SetGravityFactor(_getBodyId(), value);
    }

}