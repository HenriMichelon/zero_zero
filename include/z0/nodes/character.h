#pragma once

namespace z0 {

    class Character: public CollisionObject, 
                     public JPH::BroadPhaseLayerFilter,
                     public JPH::ObjectLayerFilter,
                     public JPH::BodyFilter {
    public:
        explicit Character(shared_ptr<Shape> shape,
                           uint32_t layer=1,
                           uint32_t mask=1,
                           const string& name = "Character");
        ~Character() override;

        bool isOnGround();
        bool isGround(CollisionObject*);
        void setVelocity(vec3 velocity) override;
        vec3 getVelocity() const override;

    protected:
        void setPositionAndRotation();

    private:
        unique_ptr<JPH::CharacterVirtual> character;
        //unique_ptr<JPH::Character> subCharacter;
        
    public:
        void _physicsUpdate(float delta) override;

         bool ShouldCollide (JPH::BroadPhaseLayer inLayer) const override {
            return true;
        };
        bool ShouldCollide (JPH::ObjectLayer inLayer) const override;
        bool ShouldCollide (const JPH::BodyID &inBodyID) const;
        bool ShouldCollideLocked (const JPH::Body &inBody) const;
    };

}