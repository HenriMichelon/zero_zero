#include "z0/z0.h"
#include "z0/resources/shape.h"

namespace z0 {

    BoxShape::BoxShape(vec3 sizes, const string& resName):
        Shape {new JPH::BoxShape(JPH::Vec3(sizes.x/2, sizes.y/2, sizes.z/2)), resName} {
    }

}