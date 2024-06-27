#pragma once

namespace z0 {

    /**
     * A 3D physics body specialized for characters moved by code
     */
    class Character: public CollisionObject, 
                     public JPH::BroadPhaseLayerFilter,
                     public JPH::ObjectLayerFilter,
                     public JPH::BodyFilter,
                     public JPH::CharacterContactListener {
    public:
        /**
         * Creates a Character with a given collision `shape`, belonging to the `layer` layers and detecting collisions with bodies having a layer in the `mask` value.
         */
        explicit Character(shared_ptr<Shape> shape,
                           uint32_t layer,
                           uint32_t mask,
                           const string& name = "Character");
        ~Character() override;

        /**
         * Returns `true` if the Character is on a ground
         */
        [[nodiscard]] bool isOnGround();

        /**
         * Returns `true` if `object` is the ground
         */
        [[nodiscard]] bool isGround(CollisionObject*object);

        /**
         * Returns the velocity in the world space of the ground.
         */
        [[nodiscard]] vec3 getGroundVelocity() const;

        /**
         * Returns the UP axis for this Character
         */
        [[nodiscard]] inline const vec3& getUpVector() const { return upVector; }

        /**
         * Sets the UP axis for this Character
         */
        void setUpVector(vec3 v);

        /**
        * Returns the list of the currently colliding bodies
        */
        [[nodiscard]] list<Collision> getCollisions() const;
        

        void setVelocity(vec3 velocity) override;
        [[nodiscard]] vec3 getVelocity() const override;

    protected:
        void setPositionAndRotation() override;

    private:
        vec3 upVector{AXIS_UP};
        unique_ptr<JPH::CharacterVirtual> character;
        unique_ptr<JPH::Character> physicsCharacter;
        
    public:
        void _physicsUpdate(float delta) override;

        void OnContactAdded(const JPH::CharacterVirtual *inCharacter, 
                            const JPH::BodyID &inBodyID2, 
                            const JPH::SubShapeID &inSubShapeID2, 
                            JPH::RVec3Arg inContactPosition, 
                            JPH::Vec3Arg inContactNormal, 
                            JPH::CharacterContactSettings &ioSettings) override;
        inline bool ShouldCollide (JPH::BroadPhaseLayer inLayer) const override {
            return true;
        };
        bool ShouldCollide (JPH::ObjectLayer inLayer) const override;
        bool ShouldCollide (const JPH::BodyID &inBodyID) const;
        bool ShouldCollideLocked (const JPH::Body &inBody) const;
    };

}