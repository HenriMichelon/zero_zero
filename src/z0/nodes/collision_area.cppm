module;
#include "z0/jolt.h"
#include "z0/libraries.h"
#include <glm/gtx/quaternion.hpp>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

export module Z0:CollisionArea;

import :CollisionObject;
import :Shape;

export namespace z0 {

    /**
     * Collision sensor that reports contacts with other bodies.
     */
    class CollisionArea: public CollisionObject {
    public:
        /**
         * Creates a CollisionArea using the given geometric `shape` to detect collision with bodies having a layer in the `mask` value.
         */
        CollisionArea(shared_ptr<Shape> _shape,
                    const uint32_t mask,
                    const string& name = "CollisionArea"):
            CollisionObject{_shape, 0, mask, name} {
            const auto position = getPositionGlobal();
            const auto quat = normalize(toQuat(mat3(worldTransform)));
            shape->setAttachedToNode();
            JPH::BodyCreationSettings settings{
                shape->_getShapeSettings(),
                JPH::RVec3{position.x, position.y, position.z},
                JPH::Quat{quat.x, quat.y, quat.z, quat.w},
                JPH::EMotionType::Dynamic,
                collisionLayer << 4 | collisionMask
            };
            settings.mIsSensor = true;
            settings.mCollideKinematicVsNonDynamic = true;
            settings.mGravityFactor = 0.0f;
            setBodyId(bodyInterface.CreateAndAddBody(settings, JPH::EActivation::DontActivate));
        }

        ~CollisionArea() override {
            bodyInterface.RemoveBody(_getBodyId());
            bodyInterface.DestroyBody(_getBodyId());
        }

    };

}