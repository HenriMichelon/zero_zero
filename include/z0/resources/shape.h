#pragma once

namespace z0 {

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
        JPH::ShapeSettings* _getShapeSettings() { return shapeSettings; }
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

}