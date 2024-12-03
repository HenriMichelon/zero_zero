/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include "z0/libraries.h"

module z0.MeshShape;

import z0.Node;
import z0.MeshInstance;
import z0.Shape;
import z0.Tools;

namespace z0 {

    MeshShape::MeshShape(const shared_ptr<Node> &node, const string &resName ):
        Shape{resName} {
        tryCreateShape(node);
    }

    MeshShape::MeshShape(const Node &node, const string &resName):
        Shape{resName} {
        const auto& meshInstance = node.findFirstChild<MeshInstance>();
        if (meshInstance != nullptr) {
            createShape(meshInstance);
        } else {
            die("MeshShape : Node ", node.toString(), "does not have a MeshInstance child");
        }
    }

    void MeshShape::tryCreateShape(const shared_ptr<Node>& node) {
        auto meshInstance = dynamic_pointer_cast<MeshInstance>(node);
        if (meshInstance == nullptr) {
            meshInstance = node->findFirstChild<MeshInstance>();
        }
        if (meshInstance != nullptr) {
            createShape(meshInstance);
        }
    }

    void MeshShape::createShape(const shared_ptr<MeshInstance>& meshInstance) {
        const auto & vertices  = meshInstance->getMesh()->getVertices();
        JPH::VertexList vertexList;
        vertexList.reserve(vertices.size());
        for (const auto &vertex : vertices) {
            vertexList.push_back(JPH::Float3{vertex.position.x, vertex.position.y, vertex.position.z});
        }

        const auto & indices = meshInstance->getMesh()->getIndices();
        JPH::IndexedTriangleList triangles;
        triangles.reserve(indices.size()/3);
        for (int i = 0; i < indices.size(); i += 3) {
            triangles.push_back({
                    indices[i + 0],
                    indices[i + 1],
                    indices[i + 2]
            });
        }

        // auto tStart = chrono::high_resolution_clock::now();
        shapeSettings = new JPH::MeshShapeSettings(vertexList, triangles);
        // auto last_time = chrono::duration<float, milli>(chrono::high_resolution_clock::now() - tStart).count();
        // log("MeshShape createShape time", meshInstance->getName(), to_string(last_time), to_string(indices.size() / 3));
    }

}
