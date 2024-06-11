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
#include "z0/nodes/physics_body.h"
#endif

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
        /*JPH::Array<JPH::Vec3> box;
        box.push_back(JPH::Vec3(5, 5, 5));
        box.push_back(JPH::Vec3(-5, 5, 5));
        box.push_back(JPH::Vec3(5, -5, 5));
        box.push_back(JPH::Vec3(-5, -5, 5));
        box.push_back(JPH::Vec3(5, 5, -5));
        box.push_back(JPH::Vec3(-5, 5, -5));
        box.push_back(JPH::Vec3(5, -5, -5));
        box.push_back(JPH::Vec3(-5, -5, -5));
        auto st = new JPH::ConvexHullShapeSettings(box);
        JPH::Shape::ShapeResult result;
        auto shape = new JPH::ConvexHullShape(*st, result);*/
        const JPH::BodyCreationSettings settings{
                _shape->_getShapeSettings(),
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