#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/resources/image.h"
#include "z0/resources/texture.h"
#include "z0/resources/material.h"
#include "z0/resources/mesh.h"
#include "z0/resources/shape.h"
#include "z0/resources/sub_shape.h"
#include "z0/nodes/mesh_instance.h"
#include "z0/resources/static_compound_shape.h"
#endif

namespace z0 {

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