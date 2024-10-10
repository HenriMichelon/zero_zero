module;
#include "z0/libraries.h"

module z0;

import :AABB;
import :Mesh;

namespace z0 {

    AABB::AABB(const Mesh* mesh) {
        auto min = vec3{numeric_limits<float>::max()};
        auto max = vec3{numeric_limits<float>::lowest()};
        const auto& vertices = mesh->getVertices();
        for (auto index : mesh->getIndices()) {
            const auto& position = vertices[index].position;
            //Get the smallest vertex
            min.x = std::min(min.x, position.x);    // Find smallest x value in model
            min.y = std::min(min.y, position.y);    // Find smallest y value in model
            min.z = std::min(min.z, position.z);    // Find smallest z value in model

            //Get the largest vertex
            max.x = std::max(max.x, position.x);    // Find largest x value in model
            max.y = std::max(max.y, position.y);    // Find largest y value in model
            max.z = std::max(max.z, position.z);    // Find largest z value in model
            // min = glm::min(min, vertex.position);
            // max = glm::max(max, vertex.position);
        }
        center = {(max + min) * 0.5f};
        extents = { max.x - center.x, max.y - center.y, max.z - center.z};
    }
}