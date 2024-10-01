module;
#include "z0/jolt.h"
#include "z0/libraries.h"
#include <glm/gtx/quaternion.hpp>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

export module Z0:PhysicsBody;

import :Tools;
import :CollisionObject;
import :Shape;
import :ConvexHullShape;

export namespace z0 {

    /**
     * Base class for 3D game objects affected by physics.
     */
    class PhysicsBody : public CollisionObject {
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
        PhysicsBody(const shared_ptr<Shape> &shape,
                    const uint32_t           layer,
                    const uint32_t           mask,
                    const JPH::EActivation   activationMode,
                    const JPH::EMotionType   motionType,
                    const string &           name,
                    const Type               type = PHYSICS_BODY):
            CollisionObject{shape, layer, mask, name, type},
            motionType{motionType} {
            this->activationMode = activationMode;
            setShape(shape);
        }

        PhysicsBody(const uint32_t         layer,
                    const uint32_t         mask,
                    const JPH::EActivation activationMode,
                    const JPH::EMotionType motionType,
                    const string &         name,
                    const Type             type = PHYSICS_BODY):
            CollisionObject{layer, mask, name, type},
            motionType{motionType} {
            this->activationMode = activationMode;
        }

        void setShape(const shared_ptr<Shape> &shape) {
            const auto &position = getPositionGlobal();
            const auto &quat     = normalize(toQuat(mat3(worldTransform)));
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

        void setProperty(const string &property, const string &value) override {
            CollisionObject::setProperty(property, value);
            if (property == "shape") {
                // split shape class name from parameters
                const auto &parts = split(value, ';');
                // we must have at least a class name
                if (parts.size() > 0) {
                    if (parts[0] == "ConvexHullShape") {
                        if (parts.size() > 2)
                            die("Missing parameter for ConvexHullShape");
                        // get the children who provide the mesh for the shape
                        const auto mesh = getChild(parts[1].data());
                        if (mesh == nullptr)
                            die("Child with path", parts[1].data(), "not found in", toString());
                        setShape(make_shared<ConvexHullShape>(mesh));
                    } else if (parts[0] == "BoxShape") {
                        if (parts.size() > 2)
                            die("Missing parameter for BoxShape");
                        setShape(make_shared<BoxShape>(to_vec3(parts[1].data())));
                    }
                }
            }
        }

    private:
        JPH::EMotionType motionType;
    };

}
