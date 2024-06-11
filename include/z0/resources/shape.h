#pragma once

namespace z0 {

    class Shape : public Resource {
    protected:
        explicit Shape(JPH::Shape* _shape, const string& resName): Resource{resName}, shape{_shape} {};
        explicit Shape(const string& resName): Resource{resName}, shape{nullptr} {};

    private:
        JPH::Shape* shape;

    public:
        JPH::Shape* _getShape() { return shape; }
    };

    class BoxShape : public Shape {
    public:
        explicit BoxShape(vec3 sizes, const string& resName = "BoxShape");
    };

    class ConvexHullShape : public Shape {
    public:
        explicit ConvexHullShape(const vector<Vertex>&, const string& resName = "BoxShape");
    private:
        vector<vec3> findConvexHull(const vector<Vertex>&);
        void quickHull(const vector<vec3>& points, vector<vec3>& hull, const vec3& P, const vec3& Q);
    };

}