/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

#include "z0/libraries.h"

module z0.resources.Shape;

import z0.Tools;

import z0.nodes.MeshInstance;

import z0.resources.Resource;

namespace z0 {

    Shape::Shape(const string &resName):
        Resource{resName} {
    }

    AABBShape::AABBShape(const Node &node, const string &resName ): Shape{resName} {
        const auto& meshInstance = node.findFirstChild<MeshInstance>();
        if (meshInstance && meshInstance->isValid()) {
            const auto& aabb = meshInstance->getMesh()->getAABB();
            const auto& extends = (aabb.max - aabb.min) * 0.5f;
            shapeSettings = new JPH::BoxShapeSettings(JPH::Vec3(extends.x, extends.y, extends.z));
        } else {
            die("AABBShape : Node ", node.toString(), "does not have a MeshInstance child");
        }
    }

    AABBShape::AABBShape(const shared_ptr<Node> &node, const string &resName) : AABBShape{*node, resName} {}

    BoxShape::BoxShape(const vec3& extends, const string &resName):
        Shape{resName}, extends
        {extends} {
        if (extends.x <= 0.2 || extends.y <= 0.2 || extends.z <= 0.2) { die("Invalid extends for BoxShape", resName); }
        shapeSettings = new JPH::BoxShapeSettings(JPH::Vec3(extends.x / 2, extends.y / 2, extends.z / 2));
    }

    shared_ptr<Resource> BoxShape::duplicate() const {
        auto dup = make_shared<BoxShape>(extends, name);
        dup->shapeSettings = new JPH::BoxShapeSettings(JPH::Vec3(extends.x / 2, extends.y / 2, extends.z / 2));
        return dup;
    }

    SphereShape::SphereShape(const float radius, const string &resName):
        Shape{resName} {
        shapeSettings = new JPH::SphereShapeSettings(radius);
    }

    CylinderShape::CylinderShape(const float radius, const float height, const string &resName):
        Shape{resName} {
        shapeSettings = new JPH::CylinderShapeSettings(height / 2.0f, radius);
    }
}
