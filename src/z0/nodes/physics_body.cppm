module;
#include "z0/jolt.h"
#include "z0/libraries.h"
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
        ~PhysicsBody() override {
            if (!_getBodyId().IsInvalid()) {
                bodyInterface.RemoveBody(_getBodyId());
                bodyInterface.DestroyBody(_getBodyId());
            }
        }

        /**
         * Sets an artificial gravity factor
         */
        void setGravityScale(const float value) {
            assert(!_getBodyId().IsInvalid());
            bodyInterface.SetGravityFactor(_getBodyId(), value);
        }

    protected:
        PhysicsBody(shared_ptr<Shape>& _shape,
                    const uint32_t layer,
                    const uint32_t mask,
                    const JPH::EActivation _activationMode,
                    const JPH::EMotionType _motionType,
                    const string& name):
            CollisionObject{_shape, layer, mask, name},
            motionType{_motionType} {
            activationMode = _activationMode;
            setShape(shape);
        }

        PhysicsBody(const uint32_t layer,
                    const uint32_t mask,
                    const JPH::EActivation _activationMode,
                    const JPH::EMotionType _motionType,
                    const string& name):
            CollisionObject{layer, mask, name},
            motionType{_motionType} {
            activationMode = _activationMode;
        }

        void setShape(shared_ptr<Shape> shape) {
            const auto position = getPositionGlobal();
            const auto quat = normalize(toQuat(mat3(worldTransform)));
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

    private:
        JPH::EMotionType motionType;
    };

}