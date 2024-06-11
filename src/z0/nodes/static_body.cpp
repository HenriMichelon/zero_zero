#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/resources/image.h"
#include "z0/resources/texture.h"
#include "z0/resources/material.h"
#include "z0/resources/mesh.h"
#include "z0/resources/shape.h"
#include "z0/nodes/collision_node.h"
#include "z0/nodes/physics_node.h"
#include "z0/nodes/physics_body.h"
#include "z0/nodes/static_body.h"
#endif

namespace z0 {

    StaticBody::StaticBody(shared_ptr<Shape> shape, uint32_t layer, uint32_t mask, const string& name):
        PhysicsBody(shape,
                    layer,
                    mask,
                    JPH::EActivation::DontActivate,
                    JPH::EMotionType::Static,
                    name) {
    }

}