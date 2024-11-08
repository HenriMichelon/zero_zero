/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include "z0/libraries.h"

module z0.ConvexHullShape;

import z0.Mesh;
import z0.MeshInstance;
import z0.Node;
import z0.Resource;
import z0.Tools;

namespace z0 {

    ConvexHullShape::ConvexHullShape(const shared_ptr<Node> &node, const string &resName):
        Shape{resName} {
        tryCreateShape(node);
    }

    ConvexHullShape::ConvexHullShape(const shared_ptr<Mesh> &mesh, const string &resName):
        Shape{resName} {
        createShape(mesh);
    }

    void ConvexHullShape::tryCreateShape(const shared_ptr<Node> &node) {
        auto meshInstance = dynamic_pointer_cast<MeshInstance>(node);
        if (meshInstance == nullptr) {
            const auto& meshNode = node->findFirstChild<MeshInstance>();
            if (meshNode != nullptr) meshInstance = meshNode;
        }
        if (meshInstance != nullptr) {
            createShape(meshInstance);
        }
    }

    ConvexHullShape::ConvexHullShape(const vector<vec3>& points, const string &resName):
    Shape{resName}, points{points} {
        createShape();
    }

    shared_ptr<Resource> ConvexHullShape::duplicate() const {
        return make_shared<ConvexHullShape>(points, name);
    }

    void ConvexHullShape::createShape(const shared_ptr<MeshInstance>& meshInstance) {
        points.clear();
        const auto &transform = meshInstance->getTransformLocal();
        for (const auto &vertex : meshInstance->getMesh()->getVertices()) {
            points.push_back(transform * vec4{vertex.position, 1.0f});
        }
        createShape();
    }

    void ConvexHullShape::createShape(const shared_ptr<Mesh> &mesh) {
        points.clear();
        for (const auto &vertex : mesh->getVertices()) {
            points.push_back(vertex.position);
        }
        createShape();
    }

    void ConvexHullShape::createShape() {
        JPH::Array<JPH::Vec3> jphPoints;
        for (const auto &vertex : points) {
            jphPoints.push_back(JPH::Vec3{vertex.x, vertex.y, vertex.z});
        }
        shapeSettings = new JPH::ConvexHullShapeSettings(jphPoints);
    }

}
