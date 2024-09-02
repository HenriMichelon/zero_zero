module;
#include <cassert>
#include "z0/jolt.h"
#include "z0/libraries.h"
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>

export module Z0:ConvexHullShape;

import :Tools;
import :Node;
import :MeshInstance;
import :Shape;

export namespace z0 {

    /**
     * A convex hull collision shape
     */
    class ConvexHullShape : public Shape {
    public:
        /**
         * Creates a ConvexHullShape using the vertices of the Mesh of first MeshInstance found in the `node` tree
         */
        explicit ConvexHullShape(Node*node, const string& resName = "ConvexHullShape");

        /**
         * Creates a ConvexHullShape using the vertices of the Mesh of the first MeshInstance found in the `node` tree
         */
        explicit ConvexHullShape(const shared_ptr<Node>&node, const string& resName = "ConvexHullShape");

    private:
        void tryCreateShape(Node*);
        void createShape(const MeshInstance*);
    };

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