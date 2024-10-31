module;
#include "z0/jolt.h"
#include "z0/libraries.h"
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

module z0;

import :Tools;
import :Resource;
import :Shape;

namespace z0 {

    Shape::Shape(const string &resName):
        Resource{resName} {
    }

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

}
