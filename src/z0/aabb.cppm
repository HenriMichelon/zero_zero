/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.AABB;

export namespace z0 {

    /**
    * A 3D axis-aligned bounding box.
    */
    struct AABB {
        vec3 min{};
        vec3 max{};

        AABB() = default;

        inline AABB(const vec3& min, const vec3& max) : min{min}, max{max} {}

        AABB toGlobal(const mat4& transform) const;

    };
}