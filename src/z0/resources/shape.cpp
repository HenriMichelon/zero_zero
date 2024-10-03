module;
#include "z0/jolt.h"
#include "z0/libraries.h"
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

module z0;

import :Tools;
import :Shape;

namespace z0 {

    Shape::Shape(const string &resName):
        Resource{resName} {
    }

    void Shape::setAttachedToNode() {
        if (isAttachedToNode) { die("Shape already attached to a node"); }
        isAttachedToNode = true;
    }

    BoxShape::BoxShape(const vec3 extends, const string &resName):
        Shape{resName} {
        shapeSettings = new JPH::BoxShapeSettings(JPH::Vec3(extends.x / 2, extends.y / 2, extends.z / 2));
    }

    SphereShape::SphereShape(const float radius, const string &resName):
        Shape{resName} {
        shapeSettings = new JPH::SphereShapeSettings(radius);
    }

}
