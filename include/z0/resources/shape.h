#pragma once

namespace z0 {

    class Shape : public Resource {
    public:
        void setAttachedToNode();

    protected:
        explicit Shape(const string& resName): Resource{resName}, shapeSettings{nullptr} {};

    protected:
        JPH::ShapeSettings* shapeSettings{nullptr};

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