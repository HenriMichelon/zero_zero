#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/resources/image.h"
#include "z0/resources/texture.h"
#include "z0/resources/material.h"
#include "z0/resources/mesh.h"
#include "z0/resources/shape.h"
#endif

namespace z0 {

    void Shape::setAttachedToNode() { 
        if (isAttachedToNode) { die("Shape already attached to a node"); }
        isAttachedToNode = true; 
    }

    BoxShape::BoxShape(vec3 sizes, const string& resName):
        Shape {resName} {
        shapeSettings = new JPH::BoxShapeSettings(JPH::Vec3(sizes.x/2, sizes.y/2, sizes.z/2));
    }

}