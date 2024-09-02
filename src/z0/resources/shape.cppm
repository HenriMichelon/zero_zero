module;
#include "z0/jolt.h"
#include "z0/modules.h"
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

export module Z0:Shape;

import :Tools;
import :Resource;

export namespace z0 {

    /**
     * Base class for all collision shapes
     */
    class Shape : public Resource {
    protected:
        JPH::ShapeSettings* shapeSettings{nullptr};
        explicit Shape(const string& resName): Resource{resName}, shapeSettings{nullptr} {};

    private:
        bool isAttachedToNode{false};

    public:
        [[nodiscard]] JPH::ShapeSettings* _getShapeSettings() { return shapeSettings; }
        void setAttachedToNode();
    };

    /**
     * Box shaped collision Shape
     */
    class BoxShape : public Shape {
    public:
        /**
         * Creates a BoxShape with the given extends
         */
        explicit BoxShape(vec3 extends, const string& resName = "BoxShape");
    };


    /**
     * Sphere shaped collision Shape
     */
    class SphereShape : public Shape {
    public:
        /**
         * Create a SphereShape with the given radius
         */
        explicit SphereShape(float radius, const string& resName = "SphereShape");
    };

    void Shape::setAttachedToNode() {
        if (isAttachedToNode) { die("Shape already attached to a node"); }
        isAttachedToNode = true;
    }

    BoxShape::BoxShape(vec3 sizes, const string& resName):
        Shape {resName} {
        shapeSettings = new JPH::BoxShapeSettings(JPH::Vec3(sizes.x/2, sizes.y/2, sizes.z/2));
    }

    SphereShape::SphereShape(float radius, const string& resName):
        Shape {resName} {
        shapeSettings = new JPH::SphereShapeSettings(radius);
    }


}