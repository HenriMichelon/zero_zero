module;
#include "z0/jolt.h"
#include "z0/libraries.h"
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

export module z0:Shape;

import :Tools;
import :Resource;

export namespace z0 {

    /**
     * Base class for all collision shapes
     */
    class Shape : public Resource {
    protected:
        JPH::ShapeSettings *shapeSettings{nullptr};

        explicit Shape(const string &resName):
            Resource{resName} {
        }

    private:
        bool isAttachedToNode{false};

    public:
        [[nodiscard]] JPH::ShapeSettings *_getShapeSettings() const { return shapeSettings; }

        void setAttachedToNode() {
            if (isAttachedToNode) { die("Shape already attached to a node"); }
            isAttachedToNode = true;
        }
    };

    /**
     * Box shaped collision Shape
     */
    class BoxShape : public Shape {
    public:
        /**
         * Creates a BoxShape with the given extends
         */
        explicit BoxShape(const vec3 extends, const string &resName = "BoxShape"):
            Shape{resName} {
            shapeSettings = new JPH::BoxShapeSettings(JPH::Vec3(extends.x / 2, extends.y / 2, extends.z / 2));
        }
    };

    /**
     * Sphere shaped collision Shape
     */
    class SphereShape : public Shape {
    public:
        /**
         * Create a SphereShape with the given radius
         */
        explicit SphereShape(const float radius, const string &resName = "SphereShape"):
            Shape{resName} {
            shapeSettings = new JPH::SphereShapeSettings(radius);
        }
    };

}
