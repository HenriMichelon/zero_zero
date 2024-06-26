#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/resources/image.h"
#include "z0/resources/texture.h"
#include "z0/resources/material.h"
#include "z0/resources/mesh.h"
#include "z0/resources/shape.h"
#include "z0/nodes/collision_object.h"
#include "z0/nodes/physics_body.h"
#include "z0/nodes/static_body.h"
#endif

namespace z0 {

    StaticBody::StaticBody(shared_ptr<Shape> shape, uint32_t layer, const string& name):
        PhysicsBody(shape,
                    layer,
                    0,
                    JPH::EActivation::DontActivate,
                    JPH::EMotionType::Static,
                    name) {
    }

    StaticBody::StaticBody(uint32_t layer, const string& name):
        PhysicsBody(layer,
                    0,
                    JPH::EActivation::DontActivate,
                    JPH::EMotionType::Static,
                    name) {
    }

}