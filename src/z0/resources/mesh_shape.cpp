#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/resources/image.h"
#include "z0/resources/texture.h"
#include "z0/resources/material.h"
#include "z0/resources/mesh.h"
#include "z0/resources/shape.h"
#include "z0/nodes/mesh_instance.h"
#include "z0/resources/mesh_shape.h"
#endif

namespace z0 {
    
    MeshShape::MeshShape(Node *node, const string& resName):
        Shape{resName} {
        assert(node && "Invalid Node");
        tryCreateShape(node);
    }

    MeshShape::MeshShape(const shared_ptr<Node>&node, const string& resName):
            Shape{resName} {
       tryCreateShape(node.get());
    }

    void MeshShape::tryCreateShape(Node*node) {
        const auto* meshInstance = dynamic_cast<MeshInstance*>(node);
        if (meshInstance == nullptr) {
            meshInstance = node->findFirstChild<MeshInstance>();
        }
        if (meshInstance != nullptr) {
            createShape(meshInstance);
        }
    }

    void MeshShape::createShape(const MeshInstance* meshInstance) {
        JPH::VertexList vertexList;
        const auto& transform = meshInstance->getTransformLocal();
        const auto& vertices = meshInstance->getMesh()->getVertices();
        for(const auto& vertex : vertices) {
            const auto& v1 = transform * vec4{vertex.position, 1.0f};
            vertexList.push_back(JPH::Float3{v1.x, v1.y, v1.z});
        }
        JPH::IndexedTriangleList triangles;
        const auto& indices = meshInstance->getMesh()->getIndices();
        for(int i = 0; i < indices.size(); i+=3) {
            triangles.push_back({
                indices[i+0],
                indices[i+1],
                indices[i+2]
            });
        }
        shapeSettings = new JPH::MeshShapeSettings(vertexList, triangles);
    }

}