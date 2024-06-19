#pragma once

namespace z0 {

    /**
     * Sub shape composing a StaticCompoundShape
     */
    struct SubShape {
        shared_ptr<Shape> shape;
        vec3              position{0.0f};
        vec3              rotation{0.0f};
    };


}