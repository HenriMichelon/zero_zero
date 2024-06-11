#include "z0/z0.h"
#include "z0/resources/image.h"
#include "z0/resources/texture.h"
#include "z0/resources/material.h"
#include "z0/resources/mesh.h"
#include "z0/resources/shape.h"

namespace z0 {

    BoxShape::BoxShape(vec3 sizes, const string& resName):
        Shape {new JPH::BoxShape(JPH::Vec3(sizes.x/2, sizes.y/2, sizes.z/2)), resName} {}

    ConvexHullShape::ConvexHullShape(const vector<Vertex>&vertices, const string& resName = "BoxShape"):
        Shape{resName} {
        JPH::Array<JPH::Vec3> points;
        for(const auto& point : findConvexHull(vertices)) {
            points.push_back(JPH::Vec3{point.x, point.y, point.z});
        }
        JPH::ConvexHullShapeSettings settings{points};
        JPH::Shape::ShapeResult result;
        shape = new JPH::ConvexHullShape(settings, result);
    }

    float cross(const glm::vec3& O, const glm::vec3& A, const glm::vec3& B) {
        return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
    }

    void ConvexHullShape::quickHull(const vector<vec3>& points, vector<vec3>& hull, const vec3& P, const vec3& Q) {
        int idx = -1;
        float maxDist = 0.0f;

        for (int i = 0; i < points.size(); ++i) {
            float dist = cross(P, Q, points[i]);
            if (dist > maxDist) {
                idx = i;
                maxDist = dist;
            }
        }

        if (idx == -1) {
            hull.push_back(P);
            hull.push_back(Q);
            return;
        }

        quickHull(points, hull, points[idx], P);
        quickHull(points, hull, points[idx], Q);
    }

    vector<vec3> ConvexHullShape::findConvexHull(const vector<Vertex>& vertices) {
        vector<vec3> points;
        for (const auto& vertex: vertices) {
            points.push_back(vertex.position);
        }

        vector<vec3> hull;

        if (points.size() < 3) {
            return hull; // Convex hull is not possible
        }

        // Find the points with the minimum and maximum x-coordinates
        auto minMaxX = std::minmax_element(points.begin(), points.end(), [](const glm::vec3& a, const glm::vec3& b) {
            return a.x < b.x;
        });

        glm::vec3 A = *minMaxX.first;
        glm::vec3 B = *minMaxX.second;

        // Partition the points into two sets
        std::vector<glm::vec3> leftSet, rightSet;
        for (const auto& point : points) {
            if (cross(A, B, point) < 0) {
                leftSet.push_back(point);
            } else if (cross(A, B, point) > 0) {
                rightSet.push_back(point);
            }
        }

        quickHull(leftSet, hull, A, B);
        quickHull(rightSet, hull, B, A);

        // Remove duplicate points
        std::sort(hull.begin(), hull.end(), [](const glm::vec3& a, const glm::vec3& b) {
            return a.x < b.x || (a.x == b.x && a.y < b.y);
        });
        hull.erase(std::unique(hull.begin(), hull.end()), hull.end());

        return hull;
    }


}