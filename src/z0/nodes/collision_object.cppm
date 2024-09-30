module;
#include "z0/jolt.h"
#include "z0/libraries.h"
#include <glm/gtx/quaternion.hpp>

export module Z0:CollisionObject;

import :Constants;
import :Tools;
import :Node;
import :Signal;
import :Shape;
import :Application;

export namespace z0 {

    /**
     * Base class for 3D physics objects.
     * Object A can detect a contact with object B only if object B is in any of the layers that object A scans (mask value)
     */
    class CollisionObject : public Node {
    public:
        /**
         * Signal called whenever a new contact point is detected and reports the first contact point in a CollisionObject::Collision
         */
        static const Signal::signal on_collision_starts;
        /**
         * Signal called whenever a contact is detected that was also detected last update and reports the first contact point in a CollisionObject::Collision
         */
        static const Signal::signal on_collision_persists;

        /**
         * Collision data for the CollisionObject::on_collision_starts and CollisionObject::on_collision_persists signal
         */
        struct Collision : Signal::Parameters {
            /** World space position of the first contact point, on the colliding `object` */
            vec3 position;
            /** Normal for this collision, direction along which to move the `object` out of collision along the shortest path.  */
            vec3 normal;
            /** Collising object */
            CollisionObject *object;
        };

        /**
         * The physics layers this CollisionObject is in.
         */
        [[nodiscard]] uint32_t getCollisionLayer() const { return collisionLayer; }

        /**
         * The physics layers this CollisionObject scans.
         */
        [[nodiscard]] uint32_t getCollistionMask() const { return collisionMask; }

        /**
         * Returns `true` if the object is in the `layer`
         */
        [[nodiscard]] bool haveCollisionLayer(const uint32_t layer) const {
            return collisionLayer & layer;
        }

        /**
         * Returns `true` if the object have the `mask`
         */

        [[nodiscard]] bool haveCollisionMask(const uint32_t mask) const {
            return collisionMask & mask;
        }

        /**
         * Adds or removes a collision layer
         */
        virtual void setCollistionLayer(const uint32_t layer, const bool value) {
            assert(!bodyId.IsInvalid());
            if (value) {
                collisionLayer |= layer;
            } else {
                collisionLayer &= ~layer;
            }
            bodyInterface.SetObjectLayer(bodyId, collisionLayer << 4 | collisionMask);
        }

        /**
         * Adds or removes a collision mask
         */
        virtual void setCollistionMask(const uint32_t layer, const bool value) {
            assert(!bodyId.IsInvalid());
            if (value) {
                collisionMask |= layer;
            } else {
                collisionMask &= ~layer;
            }
            bodyInterface.SetObjectLayer(bodyId, collisionLayer << 4 | collisionMask);
        }

        /**
         * Returns `true` if the object can collide with an object with the layer `layer`
         */
        [[nodiscard]] bool shouldCollide(const uint32_t layer) const {
            return (layer & collisionMask) != 0;
        }

        /**
         * Sets the linear velocity
         */
        virtual void setVelocity(const vec3 velocity) {
            assert(!bodyId.IsInvalid());
            if (velocity == VEC3ZERO) {
                bodyInterface.SetLinearVelocity(bodyId, JPH::Vec3::sZero());
            } else {
                // current orientation * velocity
                const auto vel = toQuat(mat3(localTransform)) * velocity;
                bodyInterface.SetLinearVelocity(bodyId, JPH::Vec3{vel.x, vel.y, vel.z});
            }
        }

        /**
         * Returns the linear velocity
         */
        [[nodiscard]] virtual vec3 getVelocity() const {
            assert(!bodyId.IsInvalid());
            const auto velocity = bodyInterface.GetLinearVelocity(bodyId);
            return vec3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
        }

        /**
         * Add force (unit: N) at center of mass for the next time step, will be reset after the next physics update
         */
        void applyForce(const vec3 force) const {
            assert(!bodyId.IsInvalid());
            bodyInterface.AddForce(
                    bodyId,
                    JPH::Vec3{force.x, force.y, force.z});
        }

        /**
         * Add force (unit: N) at `position` for the next time step, will be reset after the next physics update
         */
        void applyForce(const vec3 force, const vec3 position) const {
            assert(!bodyId.IsInvalid());
            bodyInterface.AddForce(
                    bodyId,
                    JPH::Vec3{force.x, force.y, force.z},
                    JPH::Vec3{position.x, position.y, position.z});
        }

        /**
         * Returns `true` if `obj` were in contact with the object during the last simulation step
         */
        [[nodiscard]] bool wereInContact(const CollisionObject *obj) const {
            assert(!bodyId.IsInvalid());
            return app()._getPhysicsSystem().WereBodiesInContact(bodyId, obj->bodyId);
        }

        void setProperty(const string &property, const string &value) override {
            Node::setProperty(property, value);
            if (property == "layer") {
                setCollistionLayer(stoul(value), true);
            } else if (property == "mask") {
                setCollistionMask(stoul(value), true);
            }
        }

    protected:
        bool                updating{false};
        uint32_t            collisionLayer;
        uint32_t            collisionMask;
        shared_ptr<Shape>   shape;
        JPH::EActivation    activationMode;
        JPH::BodyInterface &bodyInterface;

        CollisionObject(const shared_ptr<Shape> &_shape,
                        const uint32_t           layer,
                        const uint32_t           mask,
                        const string &           name,
                        const Type               type = COLLISION_OBJECT):
            Node{name, type},
            collisionLayer{layer},
            collisionMask{mask},
            shape{_shape},
            activationMode{JPH::EActivation::Activate},
            bodyInterface{app()._getBodyInterface()} {
        }

        CollisionObject(const uint32_t layer,
                        const uint32_t mask,
                        const string & name,
                        const Type     type = COLLISION_OBJECT):
            Node{name, type},
            collisionLayer{layer},
            collisionMask{mask},
            shape{nullptr},
            activationMode{JPH::EActivation::Activate},
            bodyInterface{app()._getBodyInterface()} {
        }

        virtual void setPositionAndRotation() {
            if (updating || bodyId.IsInvalid())
                return;
            const auto position = getPositionGlobal();
            const auto quat     = normalize(toQuat(mat3(worldTransform)));
            bodyInterface.SetPositionAndRotation(
                    bodyId,
                    JPH::RVec3(position.x, position.y, position.z),
                    JPH::Quat(quat.x, quat.y, quat.z, quat.w),
                    activationMode);
        }

        void setBodyId(const JPH::BodyID id) {
            bodyId = id;
            bodyInterface.SetUserData(bodyId, reinterpret_cast<uint64>(this));
            //log(toString(), " body id ", to_string(id.GetIndexAndSequenceNumber()));
        }

        [[nodiscard]] CollisionObject *_getByBodyId(const JPH::BodyID id) const {
            assert(!bodyId.IsInvalid());
            return reinterpret_cast<CollisionObject *>(bodyInterface.GetUserData(id));
        }

        void _updateTransform() override {
            Node::_updateTransform();
            setPositionAndRotation();
        }

        void _updateTransform(const mat4 &parentMatrix) override {
            Node::_updateTransform(parentMatrix);
            setPositionAndRotation();
        }

    private:
        JPH::BodyID bodyId{JPH::BodyID::cInvalidBodyID};

    public:
        void _physicsUpdate(const float delta) override {
            assert(!bodyId.IsInvalid());
            Node::_physicsUpdate(delta);
            updating = true;
            JPH::Vec3 position;
            JPH::Quat rotation;
            bodyInterface.GetPositionAndRotation(bodyId, position, rotation);
            const auto pos = vec3{position.GetX(), position.GetY(), position.GetZ()};
            if (pos != getPositionGlobal()) {
                setPositionGlobal(pos);
            }
            setRotation(quat{rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ()});
            updating = false;
        }

        void _onEnterScene() override {
            bodyInterface.ActivateBody(bodyId);
            setPositionAndRotation();
            Node::_onEnterScene();
        }

        void _onExitScene() override {
            bodyInterface.DeactivateBody(bodyId);
            Node::_onExitScene();
        }

        void _onPause() override {
            if (!bodyId.IsInvalid()) {
                bodyInterface.DeactivateBody(bodyId);
            }
        }

        void _onResume() override {
            if (!bodyId.IsInvalid()) {
                bodyInterface.ActivateBody(bodyId);
            }
        }

        [[nodiscard]] JPH::BodyID _getBodyId() const { return bodyId; }

        inline ~CollisionObject() override = default;
    };

    const Signal::signal CollisionObject::on_collision_starts   = "on_collision_starts";
    const Signal::signal CollisionObject::on_collision_persists = "on_collision_persists";

}
