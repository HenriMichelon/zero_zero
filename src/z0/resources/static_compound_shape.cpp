module;
#include "z0/jolt.h"
#include "z0/libraries.h"
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>

module z0;

import :Tools;
import :Shape;
import :SubShape;

namespace z0 {

    StaticCompoundShape::StaticCompoundShape(const vector<SubShape> &subshapes, const string &resName) :
        Shape{resName} {
        const auto settings = new JPH::StaticCompoundShapeSettings();
        for (const auto &subshape : subshapes) {
            const auto quat = glm::quat(subshape.rotation);
            settings->AddShape(JPH::Vec3{subshape.position.x, subshape.position.y, subshape.position.z},
                               JPH::Quat{quat.x, quat.y, quat.z, quat.w},
                               subshape.shape->_getShapeSettings());
        }
        shapeSettings = settings;
    }

}
