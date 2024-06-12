#pragma once

namespace z0 {

    class Shape : public Resource {
    protected:
        explicit Shape(const string& resName): Resource{resName}, shapeSettings{nullptr} {};

    protected:
        JPH::ShapeSettings* shapeSettings{nullptr};

    public:
        JPH::ShapeSettings* _getShapeSettings() { return shapeSettings; }
    };

    class BoxShape : public Shape {
    public:
        explicit BoxShape(vec3 sizes, const string& resName = "BoxShape");
    };

    class ConvexHullShape : public Shape {
    public:
        explicit ConvexHullShape(Node&, const string& resName = "ConvexHullShape");
    };

}