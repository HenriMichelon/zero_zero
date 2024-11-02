/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

module z0;

import :AABB;

namespace z0 {

    // https://ktstephano.github.io/rendering/stratusgfx/aabbs
    AABB AABB::toGlobal(const mat4& transform) const {
        // First extract the 8 transformed corners of the box using vmin/vmax
        const auto corner = vec3{transform * vec4{min.x, min.y, min.z, 1.0f}};
        const vec3 corners[8] = {
            vec3(transform * vec4(min.x, min.y, min.z, 1.0f)),
            vec3(transform * vec4(min.x, max.y, min.z, 1.0f)),
            vec3(transform * vec4(min.x, min.y, max.z, 1.0f)),
            vec3(transform * vec4(min.x, max.y, max.z, 1.0f)),
            vec3(transform * vec4(max.x, min.y, min.z, 1.0f)),
            vec3(transform * vec4(max.x, max.y, min.z, 1.0f)),
            vec3(transform * vec4(max.x, min.y, max.z, 1.0f)),
            vec3(transform * vec4(max.x, max.y, max.z, 1.0f))
        };

        // Now apply the min/max algorithm from before using the 8 transformed
        // corners
        auto newVmin = corners[0];
        auto newVmax = newVmin;

        // Start looping from corner 1 onwards
        for (auto i = 1; i < 8; ++i) {
            const glm::vec3& current = corners[i];
            newVmin = glm::min(newVmin, current);
            newVmax = glm::max(newVmax, current);
        }

        // Now pack them into our new bounding box
        return { newVmin, newVmax };
    }

}