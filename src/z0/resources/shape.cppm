module;
#include "z0/jolt.h"
#include "z0/libraries.h"
#include <Jolt/Physics/Collision/Shape/Shape.h>

export module z0:Shape;

import :Resource;

export namespace z0 {

    /**
     * Base class for all collision shapes
     */
    class Shape : public Resource {
    protected:
        JPH::ShapeSettings *shapeSettings{nullptr};

        explicit Shape(const string &resName);

    private:
        bool isAttachedToNode{false};

    public:
        [[nodiscard]] inline JPH::ShapeSettings *_getShapeSettings() const { return shapeSettings; }

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
        explicit BoxShape(vec3 extends, const string &resName = "BoxShape");
    };

    /**
     * Sphere shaped collision Shape
     */
    class SphereShape : public Shape {
    public:
        /**
         * Create a SphereShape with the given radius
         */
        explicit SphereShape(float radius, const string &resName = "SphereShape");
    };

}
