#pragma once

namespace z0 {

    class Shape : public Resource {
    protected:
        explicit Shape(JPH::Shape* _shape, const string& resName): Resource{resName}, shape{_shape} {};

    private:
        JPH::Shape* shape;

    public:
        JPH::Shape* _getShape() { return shape; }
    };

    class BoxShape : public Shape {
    public:
        explicit BoxShape(vec3 sizes, const string& resName = "BoxShape");
    };

}