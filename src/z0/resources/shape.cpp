#include "z0/z0.h"
#include "z0/resources/image.h"
#include "z0/resources/texture.h"
#include "z0/resources/material.h"
#include "z0/resources/mesh.h"
#include "z0/resources/shape.h"

namespace z0 {

    BoxShape::BoxShape(vec3 sizes, const string& resName):
        Shape {resName} {
        shapeSettings = new JPH::BoxShapeSettings(JPH::Vec3(sizes.x/2, sizes.y/2, sizes.z/2));
    }

    ConvexHullShape::ConvexHullShape(const vector<Vertex>&vertices, const string& resName):
        Shape{resName} {
        JPH::Array<JPH::Vec3> points;
        for(const auto& point : vertices) {
            auto pos = 
            points.push_back(JPH::Vec3{point.position.x, point.position.y, point.position.z});
        }
        shapeSettings = new JPH::ConvexHullShapeSettings(points);
        /*JPH::Array<JPH::Vec3> box;
        box.push_back(JPH::Vec3(5, 5, 5));
        box.push_back(JPH::Vec3(-5, 5, 5));
        box.push_back(JPH::Vec3(5, -5, 5));
        box.push_back(JPH::Vec3(-5, -5, 5));
        box.push_back(JPH::Vec3(5, 5, -5));
        box.push_back(JPH::Vec3(-5, 5, -5));
        box.push_back(JPH::Vec3(5, -5, -5));
        box.push_back(JPH::Vec3(-5, -5, -5));
        shapeSettings = new JPH::ConvexHullShapeSettings(box);*/
    }

}