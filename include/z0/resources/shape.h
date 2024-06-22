#pragma once

namespace z0 {

    /**
     * Base class for all collision shapes
     */
    class Shape : public Resource {
    public:
        void setAttachedToNode();

    protected:
        JPH::ShapeSettings* shapeSettings{nullptr};
        explicit Shape(const string& resName): Resource{resName}, shapeSettings{nullptr} {};

    private:
        bool isAttachedToNode{false};

    public:
        JPH::ShapeSettings* _getShapeSettings() { return shapeSettings; }
    };

    /**
     * Box shaped collision Shape
     */
    class BoxShape : public Shape {
    public:
        explicit BoxShape(vec3 sizes, const string& resName = "BoxShape");
    };


    /**
     * Sphere shaped collision Shape
     */
    class SphereShape : public Shape {
    public:
        explicit SphereShape(float radius, const string& resName = "SphereShape");
    };

}