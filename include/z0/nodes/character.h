#pragma once

#include "z0/nodes/physics_node.h"

#include <Jolt/Physics/Character/Character.h>

namespace z0 {

    class Character: public PhysicsNode {
    public:
        explicit Character(shared_ptr<Shape> shape,
                           uint32_t layer=1,
                           uint32_t mask=1,
                           const string& name = "Character");
        ~Character() override;

        bool isOnGround();

    private:
        // Check for CharacterVirtual use for better walls & slopes interactions
        // https://jrouwe.github.io/JoltPhysics/index.html#character-controllers
        unique_ptr<JPH::Character> character;

    public:
        void _physicsUpdate() override;
    };

}