#pragma once

namespace z0 {

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

    class BoxShape : public Shape {
    public:
        explicit BoxShape(vec3 sizes, const string& resName = "BoxShape");
    };

}