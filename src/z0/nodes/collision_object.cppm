module;
#include "z0/jolt.h"
#include "z0/modules.h"
#include <glm/gtx/quaternion.hpp>

export module Z0:CollisionObject;

import :Constants;
import :Node;
import :Signal;
import :Shape;
import :Application;

export namespace z0 {

    /**
     * Base class for 3D physics objects.
     * Object A can detect a contact with object B only if object B is in any of the layers that object A scans (mask value)
     */
    class CollisionObject: public Node {
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
        struct Collision: public Signal::Parameters {
            /** World space position of the first contact point, on the colliding `object` */
            vec3             position;
            /** Normal for this collision, direction along which to move the `object` out of collision along the shortest path.  */
            vec3             normal;
            /** Collising object */
            CollisionObject* object;
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
        [[nodiscard]] bool haveCollisionLayer(uint32_t layer) const;

        /**
         * Returns `true` if the object have the `mask`
         */

        [[nodiscard]] bool haveCollisionMask(uint32_t mask) const;

        /**
         * Adds or removes a collision layer
         */
        virtual void setCollistionLayer(uint32_t layer, bool value);

        /**
         * Adds or removes a collision mask
         */
        virtual void setCollistionMask(uint32_t layer, bool value);

        /**
         * Returns `true` if the object can collide with an object with the layer `layer`
         */
        [[nodiscard]] bool shouldCollide(uint32_t layer) const;

        /**
         * Sets the linear velocity
         */
        virtual void setVelocity(vec3 velocity);

        /**
         * Returns the linear velocity
         */
        [[nodiscard]] virtual vec3 getVelocity() const;
        
        /**
         * Add force (unit: N) at center of mass for the next time step, will be reset after the next physics update
         */
        void applyForce(vec3 force);

        /**
         * Add force (unit: N) at `position` for the next time step, will be reset after the next physics update
         */
        void applyForce(vec3 force, vec3 position);

        /**
         * Returns `true` if `obj` were in contact with the object during the last simulation step
         */
        [[nodiscard]] bool wereInContact(CollisionObject* obj);

    protected:
        bool updating{false};
        uint32_t collisionLayer;
        uint32_t collisionMask;
        shared_ptr<Shape> shape;
        JPH::EActivation activationMode;
        JPH::BodyInterface& bodyInterface;

        CollisionObject(shared_ptr<Shape>& shape,
                    uint32_t layer,
                    uint32_t mask,
                    const string& name);
        CollisionObject(uint32_t layer,
                    uint32_t mask,
                    const string& name);
        
        virtual void setPositionAndRotation();
        void setBodyId(JPH::BodyID id);
        [[nodiscard]] CollisionObject* _getByBodyId(JPH::BodyID id);
        void _updateTransform() override;
        void _updateTransform(const mat4& parentMatrix) override;

    private:
        JPH::BodyID bodyId{JPH::BodyID::cInvalidBodyID};

    public:
        void _physicsUpdate(float delta) override;
        void _onEnterScene() override;
        void _onExitScene() override;
        void _onPause() override;
        void _onResume() override;

        [[nodiscard]] JPH::BodyID _getBodyId() const { return bodyId; }
        inline virtual ~CollisionObject() override = default;
    };

     const Signal::signal CollisionObject::on_collision_starts = "on_collision_starts";
    const Signal::signal CollisionObject::on_collision_persists = "on_collision_persists";

    CollisionObject::CollisionObject(uint32_t layer,
                                    uint32_t mask,
                                    const string& name):
        Node{name},
        collisionLayer{layer},
        collisionMask{mask},
        shape{nullptr},
        activationMode{JPH::EActivation::Activate},
        bodyInterface{app()._getBodyInterface()} {
    }

    CollisionObject::CollisionObject(shared_ptr<Shape>& _shape,
                             uint32_t layer,
                             uint32_t mask,
                             const string& name):
        Node{name},
        collisionLayer{layer},
        collisionMask{mask},
        shape{_shape},
        activationMode{JPH::EActivation::Activate},
        bodyInterface{app()._getBodyInterface()} {
    }

    void CollisionObject::_updateTransform() {
        Node::_updateTransform();
        setPositionAndRotation();
    }

    void CollisionObject::_updateTransform(const mat4 &parentMatrix) {
        Node::_updateTransform(parentMatrix);
        setPositionAndRotation();
    }

    CollisionObject* CollisionObject::_getByBodyId(JPH::BodyID id) {
        assert(!bodyId.IsInvalid());
        return reinterpret_cast<CollisionObject*>(bodyInterface.GetUserData(id));
    }

    bool CollisionObject::shouldCollide(uint32_t layer) const {
        return  (layer & collisionMask) != 0;
    }

    bool CollisionObject::haveCollisionLayer(uint32_t layer) const {
        return collisionLayer & layer;
    }

    bool CollisionObject::haveCollisionMask(uint32_t layer) const {
        return collisionMask & layer;
    }

    void CollisionObject::setCollistionLayer(uint32_t layer, bool value) {
        assert(!bodyId.IsInvalid());
          if (value) {
            collisionLayer |= layer;
        } else {
            collisionLayer &= ~layer;
        }
        bodyInterface.SetObjectLayer(bodyId, collisionLayer << 4 | collisionMask);
    }

    void CollisionObject::setCollistionMask(uint32_t layer, bool value) {
        assert(!bodyId.IsInvalid());
          if (value) {
            collisionMask |= layer;
        } else {
            collisionMask &= ~layer;
        }
        bodyInterface.SetObjectLayer(bodyId, collisionLayer << 4 | collisionMask);
    }

    void CollisionObject::setVelocity(vec3 velocity) {
        assert(!bodyId.IsInvalid());
        if (velocity == VEC3ZERO) {
            bodyInterface.SetLinearVelocity(bodyId, JPH::Vec3::sZero());
        } else {
            // current orientation * velocity
            velocity = toQuat(mat3(localTransform)) * velocity;
            bodyInterface.SetLinearVelocity(bodyId, JPH::Vec3{velocity.x, velocity.y, velocity.z});
        }
    }

    vec3 CollisionObject::getVelocity() const {
        assert(!bodyId.IsInvalid());
        auto velocity = bodyInterface.GetLinearVelocity(bodyId);
        return vec3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
    }

    void CollisionObject::setBodyId(JPH::BodyID id) {
        bodyId = id;
        bodyInterface.SetUserData(bodyId, reinterpret_cast<uint64>(this));
        //log(toString(), " body id ", to_string(id.GetIndexAndSequenceNumber()));
    }

    void CollisionObject::_onPause() {
        if (!bodyId.IsInvalid()) {
            bodyInterface.DeactivateBody(bodyId);
        }
    }

    void CollisionObject::_onResume() {
        if (!bodyId.IsInvalid()) {
            bodyInterface.ActivateBody(bodyId);
        }
    }

    void CollisionObject::setPositionAndRotation() {
        if (updating || bodyId.IsInvalid()) return;
        auto position = getPositionGlobal();
        auto quat = normalize(toQuat(mat3(worldTransform)));
        bodyInterface.SetPositionAndRotation(
                bodyId,
                JPH::RVec3(position.x, position.y, position.z),
                JPH::Quat(quat.x, quat.y, quat.z, quat.w),
                activationMode);
    }

    void CollisionObject::_onEnterScene() {
        bodyInterface.ActivateBody(bodyId);
        setPositionAndRotation();
        Node::_onEnterScene();
    }

    void CollisionObject::_onExitScene() {
        bodyInterface.DeactivateBody(bodyId);
        Node::_onExitScene();
    }

    void CollisionObject::_physicsUpdate(float delta) {
        assert(!bodyId.IsInvalid());
        Node::_physicsUpdate(delta);
        updating = true;
        JPH::Vec3 position;
        JPH::Quat rotation;
        bodyInterface.GetPositionAndRotation(bodyId, position, rotation);
        auto pos = vec3{position.GetX(), position.GetY(), position.GetZ()};
        if (pos != getPositionGlobal()) {
            setPositionGlobal(pos);
        }
        setRotation(quat{rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ()});
        updating = false;
    }

    void CollisionObject::applyForce(vec3 force) {
        assert(!bodyId.IsInvalid());
        bodyInterface.AddForce(
            bodyId,
            JPH::Vec3{force.x, force.y, force.z});
    }

    void CollisionObject::applyForce(vec3 force, vec3 position) {
        assert(!bodyId.IsInvalid());
        bodyInterface.AddForce(
            bodyId,
            JPH::Vec3{force.x, force.y, force.z},
            JPH::Vec3{position.x, position.y, position.z});
    }

    bool CollisionObject::wereInContact(CollisionObject* obj) {
        assert(!bodyId.IsInvalid());
        return app()._getPhysicsSystem().WereBodiesInContact(bodyId, obj->bodyId);
    }



}