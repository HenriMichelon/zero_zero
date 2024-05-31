#include "z0/base.h"
#include "z0/resources/shape.h"
#include "z0/nodes/physics_node.h"
#include "z0/nodes/physics_body.h"
#include "z0/nodes/character.h"

namespace z0 {

    Character::Character(shared_ptr<Shape> shape,
                         uint32_t layer,
                         uint32_t mask,
                         const string& name):
            PhysicsNode(shape,
                        layer,
                        mask,
                        name) {
        JPH::CharacterSettings settings;
        settings.mLayer = collisionLayer << 4 | collisionMask;
        settings.mShape = shape->_getShape();
        settings.mMaxSlopeAngle = radians(45.0);
        settings.mUp = JPH::Vec3{AXIS_UP.x, AXIS_UP.y, AXIS_UP.z};
        auto position = getPositionGlobal();
        auto quat = normalize(toQuat(mat3(worldTransform)));
        character = make_unique<JPH::Character>(&settings,
                                                JPH::RVec3(position.x, position.y, position.z),
                                                JPH::Quat(quat.x, quat.y, quat.z, quat.w),
                                                0,
                                                &Application::get()._getPhysicsSystem());
        character->AddToPhysicsSystem();
        setBodyId(character->GetBodyID());
    }

    Character::~Character() {
        character->RemoveFromPhysicsSystem();
    }

    bool Character::isOnGround() {
        return character->GetGroundState() == JPH::CharacterBase::EGroundState::OnGround;
    }

    bool Character::isGround(PhysicsNode *node) {
        return node->_getBodyId() == character->GetGroundBodyID();
    }

    static const float GROUND_COLLISION_TOLERANCE = 0.05f;

    void Character::_physicsUpdate() {
        character->PostSimulation(GROUND_COLLISION_TOLERANCE);
        PhysicsNode::_physicsUpdate();
    }

}
