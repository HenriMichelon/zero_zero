#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/resources/image.h"
#include "z0/resources/texture.h"
#include "z0/resources/material.h"
#include "z0/resources/mesh.h"
#include "z0/resources/shape.h"
#include "z0/nodes/mesh_instance.h"
#endif

namespace z0 {

    BoxShape::BoxShape(vec3 sizes, const string& resName):
        Shape {resName} {
        shapeSettings = new JPH::BoxShapeSettings(JPH::Vec3(sizes.x/2, sizes.y/2, sizes.z/2));
    }

    ConvexHullShape::ConvexHullShape(Node&node, const string& resName):
        Shape{resName} {
        if (const auto* meshInstance = dynamic_cast<MeshInstance*>(&node)) {
            JPH::Array<JPH::Vec3> points;
            const auto& transform = meshInstance->getTransformLocal();
            for(const auto& vertex : meshInstance->getMesh()->getVertices()) {
                auto point = transform * vec4{vertex.position, 1.0f};
                points.push_back(JPH::Vec3{point.x, point.y, point.z});
            }
            shapeSettings = new JPH::ConvexHullShapeSettings(points);
        }
    }

}