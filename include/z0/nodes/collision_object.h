#pragma once

namespace z0 {

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
        uint32_t getCollisionLayer() const { return collisionLayer; }

        /**
         * The physics layers this CollisionObject scans. 
         */
        uint32_t getCollistionMask() const { return collisionMask; }

        /**
         * Returns `true` if the object is in the `layer`
         */
        bool haveCollisionLayer(uint32_t layer) const;

        /**
         * Returns `true` if the object have the `mask`
         */

        bool haveCollisionMask(uint32_t mask) const;

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
        bool shouldCollide(uint32_t layer) const;

        /**
         * Sets the linear velocity
         */
        virtual void setVelocity(vec3 velocity);

        /**
         * Returns the linear velocity
         */
        virtual vec3 getVelocity() const;
        
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
        bool wereInContact(CollisionObject* obj);

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
        CollisionObject* _getByBodyId(JPH::BodyID id);
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

        JPH::BodyID _getBodyId() const { return bodyId; }
        ~CollisionObject() override = default;
    };


}