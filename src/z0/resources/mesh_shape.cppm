module;
#include <cassert>
#include "z0/jolt.h"
#include "z0/libraries.h"
#include <Jolt/Physics/Collision/Shape/MeshShape.h>

export module Z0:MeshShape;

import :Node;
import :MeshInstance;
import :Shape;

export namespace z0 {

    /**
     * A mesh shape, consisting of triangles. *Must* only be used with a StaticBody (like a terrain for example)
     */
    class MeshShape : public Shape {
    public:
        /**
         * Creates a MeshShape using the triangles of the Mesh of first MeshInstance found in the `node` tree
         */
        explicit MeshShape(Node* node, const string& resName = "MeshShape"):
            Shape{resName} {
            assert(node && "Invalid Node");
            tryCreateShape(node);
        }

        /**
         * Creates a MeshShape using the triangles of the Mesh of first MeshInstance found in the `node` tree
         */
        explicit MeshShape(const shared_ptr<Node>& node, const string& resName = "MeshShape"):
            Shape{resName} {
            tryCreateShape(node.get());
        }

    private:
        void tryCreateShape(Node* node) {
            const auto* meshInstance = dynamic_cast<MeshInstance*>(node);
            if (meshInstance == nullptr) {
                meshInstance = node->findFirstChild<MeshInstance>();
            }
            if (meshInstance != nullptr) {
                createShape(meshInstance);
            }
        }

        void createShape(const MeshInstance* meshInstance) {
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
    };

}