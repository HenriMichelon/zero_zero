module;
#include "z0/jolt.h"
#include "z0/libraries.h"
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>

export module Z0:StaticCompoundShape;

import :Tools;
import :Node;
import :MeshInstance;
import :Shape;
import :SubShape;

export namespace z0 {

    /**
     * Collision shape composed by a collection of SubShape
     */
    class StaticCompoundShape : public Shape {
    public:
        /**
         * Creates a StaticCompoundShape using the `subshaped` collection of Shape
         */
        StaticCompoundShape(const vector<SubShape>& subshapes, const string& resName = "StaticCompoundShape");
    };


    StaticCompoundShape::StaticCompoundShape(
                const vector<SubShape>& subshapes,
                const string& resName) :
        Shape{resName} {
        auto* settings = new JPH::StaticCompoundShapeSettings();
        for(const auto& subshape: subshapes) {
            auto quat = glm::quat(subshape.rotation);
            settings->AddShape(JPH::Vec3{subshape.position.x, subshape.position.y, subshape.position.z},
                               JPH::Quat{quat.x, quat.y, quat.z, quat.w},
                               subshape.shape->_getShapeSettings());
        }
        shapeSettings = settings;
    }

}