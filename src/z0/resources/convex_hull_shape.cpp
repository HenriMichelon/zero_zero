#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/resources/image.h"
#include "z0/resources/texture.h"
#include "z0/resources/material.h"
#include "z0/resources/mesh.h"
#include "z0/resources/shape.h"
#include "z0/nodes/mesh_instance.h"
#include "z0/resources/convex_hull_shape.h"
#endif

namespace z0 {
    
    ConvexHullShape::ConvexHullShape(Node *node, const string& resName):
        Shape{resName} {
        assert(node && "Invalid Node");
        tryCreateShape(node);
    }

    ConvexHullShape::ConvexHullShape(const shared_ptr<Node>&node, const string& resName):
            Shape{resName} {
       tryCreateShape(node.get());
    }

    void ConvexHullShape::tryCreateShape(Node*node) {
        const auto* meshInstance = dynamic_cast<MeshInstance*>(node);
        if (meshInstance == nullptr) {
            meshInstance = node->findFirstChild<MeshInstance>();
        }
        if (meshInstance != nullptr) {
            createShape(meshInstance);
        }
    }


    void ConvexHullShape::createShape(const MeshInstance* meshInstance) {
            JPH::Array<JPH::Vec3> points;
            const auto& transform = meshInstance->getTransformLocal();
            for(const auto& vertex : meshInstance->getMesh()->getVertices()) {
                auto point = transform * vec4{vertex.position, 1.0f};
                points.push_back(JPH::Vec3{point.x, point.y, point.z});
            }
            shapeSettings = new JPH::ConvexHullShapeSettings(points);
    }

}