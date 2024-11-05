/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/jolt.h"
#include "z0/libraries.h"
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>

module z0.StaticCompoundShape;

import z0.Tools;
import z0.Shape;
import z0.SubShape;

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
