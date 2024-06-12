#pragma once

namespace z0 {

    struct SubShape {
        shared_ptr<Shape> shape;
        vec3              position{0.0f};
        vec3              rotation{0.0f};
    };


}