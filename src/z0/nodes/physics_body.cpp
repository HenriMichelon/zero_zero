module;
#include "z0/jolt.h"
#include "z0/libraries.h"
#include <glm/gtx/quaternion.hpp>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

module z0;

import :Tools;
import :CollisionObject;
import :Shape;
import :ConvexHullShape;
import :PhysicsBody;

namespace z0 {


    void PhysicsBody::setGravityScale(const float value) {
        assert(!_getBodyId().IsInvalid());
        bodyInterface.SetGravityFactor(_getBodyId(), value);
    }

    PhysicsBody::PhysicsBody(const shared_ptr<Shape> &shape,
                             const uint32_t           layer,
                             const uint32_t           mask,
                             const JPH::EActivation   activationMode,
                             const JPH::EMotionType   motionType,
                             const string &           name,
                             const Type               type):
        CollisionObject{shape, layer, mask, name, type},
        motionType{motionType} {
        this->activationMode = activationMode;
        setShape(shape);
    }

    PhysicsBody::PhysicsBody(const uint32_t         layer,
                             const uint32_t         mask,
                             const JPH::EActivation activationMode,
                             const JPH::EMotionType motionType,
                             const string &         name,
                             const Type             type):
        CollisionObject{layer, mask, name, type},
        motionType{motionType} {
        this->activationMode = activationMode;
    }

    void PhysicsBody::setShape(const shared_ptr<Shape> &shape) {
        const auto &position = getPositionGlobal();
        const auto &quat     = normalize(toQuat(mat3(worldTransform)));
        this->shape = shape;
        const JPH::BodyCreationSettings settings{
                shape->_getShapeSettings(),
                JPH::RVec3{position.x, position.y, position.z},
                JPH::Quat{quat.x, quat.y, quat.z, quat.w},
                motionType,
                collisionLayer << 4 | collisionMask
        };
        setBodyId(bodyInterface.CreateAndAddBody(settings, JPH::EActivation::DontActivate));
    }

    void PhysicsBody::recreateBody() {
        setShape(dynamic_pointer_cast<Shape>(shape->duplicate()));
    }

    void PhysicsBody::setProperty(const string &property, const string &value) {
        CollisionObject::setProperty(property, value);
        if (property == "shape") {
            // split shape class name from parameters
            const auto &parts = split(value, ';');
            // we must have at least a class name
            if (parts.size() > 0) {
                if (parts[0] == "ConvexHullShape") {
                    if (parts.size() > 2) { die("Missing parameter for ConvexHullShape for", name); }
                    // get the children who provide the mesh for the shape
                    const auto mesh = getChild(parts[1].data());
                    if (mesh == nullptr) { die("Child with path", parts[1].data(), "not found in", name); }
                    if (mesh->getType() != MESH_INSTANCE) { die("Child with path", parts[1].data(), "not a MeshInstance in", name); }
                    setShape(make_shared<ConvexHullShape>(mesh, name));
                } else if (parts[0] == "BoxShape") {
                    if (parts.size() > 2) { die("Missing parameter for BoxShape for", name); }
                    setShape(make_shared<BoxShape>(to_vec3(parts[1].data()), name));
                }
            }
        }
    }

}
