/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/EActivation.h>
#include <glm/gtx/quaternion.hpp>
#include "z0/libraries.h"

module z0.CollisionArea;

import z0.Node;
import z0.Shape;
import z0.Tools;

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
        this->shape = shape;
        const auto position = getPositionGlobal();
        const auto quat = normalize(toQuat(mat3(worldTransform)));
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
        const auto body = bodyInterface.CreateBody(settings);
        setBodyId(body->GetID());
    }

    CollisionArea::~CollisionArea() {
        if (!_getBodyId().IsInvalid()) {
            if (bodyInterface.IsAdded(_getBodyId())) {
                bodyInterface.RemoveBody(_getBodyId());
            }
            bodyInterface.DestroyBody(_getBodyId());
        }
    }

    shared_ptr<Node> CollisionArea::duplicateInstance() {
        return make_shared<CollisionArea>(*this);
    }

    void CollisionArea::setProperty(const string &property, const string &value) {
        CollisionObject::setProperty(property, value);
        if (property == "shape") {
            // split shape class name from parameters
            const auto parts = split(value, ';');
            // we must have at least a class name
            if (parts.size() > 0) {
                if (parts.at(0) == "BoxShape") {
                    if (parts.size() < 2) { die("Missing parameter for BoxShape for", name); }
                    setShape(make_shared<BoxShape>(to_vec3(parts[1].data()), name));
                } else if (parts.at(0) == "SphereShape") {
                    if (parts.size() < 2) { die("Missing parameter for SphereShape for", name); }
                    setShape(make_shared<SphereShape>(stof(parts[1].data()), name));
                } else if (parts.at(0) == "CylinderShape") {
                    if (parts.size() < 3) { die("Missing parameter for CylinderShape for", name); }
                    setShape(make_shared<CylinderShape>(stof(parts[1].data()), stof(parts[2].data()), name));
                } else {
                    die("CollisionArea : missing or invalid shape for ", name);
                }
            }
        }
    }


}
