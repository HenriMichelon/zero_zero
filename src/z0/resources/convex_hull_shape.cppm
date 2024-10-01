module;
#include <cassert>
#include "z0/jolt.h"
#include "z0/libraries.h"
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>

export module Z0:ConvexHullShape;

import :Tools;
import :Node;
import :Mesh;
import :MeshInstance;
import :Shape;

export namespace z0 {

    /**
     * A convex hull collision shape
     */
    class ConvexHullShape : public Shape {
    public:
        /**
         * Creates a ConvexHullShape using the vertices of the Mesh of first MeshInstance found in the `node` tree.
         * Uses the local transform of the node when creating the shape.
         */
        explicit ConvexHullShape(Node*node, const string& resName = "ConvexHullShape"):
            Shape{resName} {
            assert(node && "Invalid Node");
            tryCreateShape(node);
        }

        /**
         * Creates a ConvexHullShape using the vertices of the Mesh of the first MeshInstance found in the `node` tree.
         * Uses the local transform of the node when creating the shape.
         */
        explicit ConvexHullShape(const shared_ptr<Node>&node, const string& resName = "ConvexHullShape"):
            Shape{resName} {
            tryCreateShape(node.get());
        }

        /**
         * Creates a ConvexHullShape using the vertices of the Mesh
         */
        explicit ConvexHullShape(const shared_ptr<Mesh>&mesh, const string& resName = "ConvexHullShape"):
            Shape{resName} {
            createShape(mesh);
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
            JPH::Array<JPH::Vec3> points;
            const auto& transform = meshInstance->getTransformLocal();
            for(const auto& vertex : meshInstance->getMesh()->getVertices()) {
                auto point = transform * vec4{vertex.position, 1.0f};
                points.push_back(JPH::Vec3{point.x, point.y, point.z});
            }
            shapeSettings = new JPH::ConvexHullShapeSettings(points);
        }

        void createShape(const shared_ptr<Mesh>& mesh) {
            JPH::Array<JPH::Vec3> points;
            for(const auto& vertex : mesh->getVertices()) {
                points.push_back(JPH::Vec3{vertex.position.x, vertex.position.y, vertex.position.z});
            }
            shapeSettings = new JPH::ConvexHullShapeSettings(points);
        }
    };

}