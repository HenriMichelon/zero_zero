#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/resources/image.h"
#include "z0/resources/texture.h"
#include "z0/resources/material.h"
#include "z0/resources/mesh.h"
#include "z0/resources/shape.h"
#include "z0/nodes/collision_node.h"
#include "z0/application.h"
#endif

namespace z0 {

    CollisionNode::CollisionNode(uint32_t layer,
                             uint32_t mask,
                             const string& name):
        Node{name},
        collisionLayer{layer},
        collisionMask{mask},
        bodyInterface{app()._getBodyInterface()} {
        needPhysics = true;
    }

    CollisionNode* CollisionNode::_getByBodyId(JPH::BodyID id) {
        return reinterpret_cast<CollisionNode*>(bodyInterface.GetUserData(id));
    }

    bool CollisionNode::shouldCollide(uint32_t layer) const {
        return  (layer & collisionMask) != 0;
    }

    bool CollisionNode::haveCollisionLayer(uint32_t layer) const {
        return collisionLayer & layer;
    }

    bool CollisionNode::haveCollisionMask(uint32_t layer) const {
        return collisionMask & layer;
    }

    void CollisionNode::setCollistionLayer(uint32_t layer, bool value) {
        if (value) {
            collisionLayer |= layer;
        } else {
            collisionLayer &= ~layer;
        }
    }

    void CollisionNode::setCollistionMask(uint32_t layer, bool value) {
        if (value) {
            collisionMask |= layer;
        } else {
            collisionMask &= ~layer;
        }
    }

}