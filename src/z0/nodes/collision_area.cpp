#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/resources/image.h"
#include "z0/resources/texture.h"
#include "z0/resources/material.h"
#include "z0/resources/mesh.h"
#include "z0/resources/shape.h"
#include "z0/nodes/collision_object.h"
#include "z0/nodes/collision_area.h"
#endif

namespace z0 {

    CollisionArea::CollisionArea(shared_ptr<Shape> _shape,
                             uint32_t mask,
                             const string& name):
            CollisionObject{_shape, 0, mask, name} {
        auto position = getPositionGlobal();
        auto quat = normalize(toQuat(mat3(worldTransform)));
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

    CollisionArea::~CollisionArea() {
        bodyInterface.RemoveBody(_getBodyId());
        bodyInterface.DestroyBody(_getBodyId());
    }

}