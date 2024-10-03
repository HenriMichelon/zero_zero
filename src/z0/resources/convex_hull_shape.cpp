module;
#include <cassert>
#include "z0/jolt.h"
#include "z0/libraries.h"
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>

module z0;

import :Tools;
import :ConvexHullShape;

namespace z0 {

    ConvexHullShape::ConvexHullShape(Node *node, const string &resName):
        Shape{resName} {
        assert(node && "Invalid Node");
        tryCreateShape(node);
    }

    ConvexHullShape::ConvexHullShape(const shared_ptr<Node> &node, const string &resName):
        Shape{resName} {
        tryCreateShape(node.get());
    }

    ConvexHullShape::ConvexHullShape(const shared_ptr<Mesh> &mesh, const string &resName):
        Shape{resName} {
        createShape(mesh);
    }

    void ConvexHullShape::tryCreateShape(Node *node) {
        const auto *meshInstance = dynamic_cast<MeshInstance *>(node);
        if (meshInstance == nullptr) {
            meshInstance = node->findFirstChild<MeshInstance>();
        }
        if (meshInstance != nullptr) {
            createShape(meshInstance);
        }
    }

    void ConvexHullShape::createShape(const MeshInstance *meshInstance) {
        JPH::Array<JPH::Vec3> points;
        const auto &          transform = meshInstance->getTransformLocal();
        for (const auto &vertex : meshInstance->getMesh()->getVertices()) {
            auto point = transform * vec4{vertex.position, 1.0f};
            points.push_back(JPH::Vec3{point.x, point.y, point.z});
        }
        shapeSettings = new JPH::ConvexHullShapeSettings(points);
    }

    void ConvexHullShape::createShape(const shared_ptr<Mesh> &mesh) {
        JPH::Array<JPH::Vec3> points;
        for (const auto &vertex : mesh->getVertices()) {
            points.push_back(JPH::Vec3{vertex.position.x, vertex.position.y, vertex.position.z});
        }
        shapeSettings = new JPH::ConvexHullShapeSettings(points);
    }

}
