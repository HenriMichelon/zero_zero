module;
#include "z0/jolt.h"
#include "z0/libraries.h"
#include <glm/gtx/quaternion.hpp>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

module z0;

import :CollisionArea;

namespace z0 {

    CollisionArea::CollisionArea(const shared_ptr<Shape> &shape,
                                 const uint32_t           mask,
                                 const string &           name):
        CollisionObject{shape, 0, mask, name, COLLISION_AREA} {
        setShape(shape);
    }

    CollisionArea::CollisionArea(const string &name):
        CollisionObject{0, 0, name, COLLISION_AREA} {
    }

    void CollisionArea::setShape(const shared_ptr<Shape> &shape) {
        this->shape         = shape;
        const auto position = getPositionGlobal();
        const auto quat     = normalize(toQuat(mat3(worldTransform)));
        this->shape->setAttachedToNode();
        JPH::BodyCreationSettings settings{
                shape->_getShapeSettings(),
                JPH::RVec3{position.x, position.y, position.z},
                JPH::Quat{quat.x, quat.y, quat.z, quat.w},
                JPH::EMotionType::Dynamic,
                collisionLayer << 4 | collisionMask
        };
        settings.mIsSensor                     = true;
        settings.mCollideKinematicVsNonDynamic = true;
        settings.mGravityFactor                = 0.0f;
        setBodyId(bodyInterface.CreateAndAddBody(settings, JPH::EActivation::DontActivate));
    }

    CollisionArea::~CollisionArea() {
        bodyInterface.RemoveBody(_getBodyId());
        bodyInterface.DestroyBody(_getBodyId());
    }

}
