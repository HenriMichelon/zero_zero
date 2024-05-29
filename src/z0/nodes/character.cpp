#include "z0/nodes/character.h"
#include "z0/application.h"

#include <glm/gtx/quaternion.hpp>

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
        bodyId = character->GetBodyID();
    }

    Character::~Character() {
        character->RemoveFromPhysicsSystem();
    }


}
