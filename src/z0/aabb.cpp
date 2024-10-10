module;
#include "z0/libraries.h"

module z0;

import :AABB;
import :Mesh;

namespace z0 {

    AABB::AABB(const Mesh* mesh) {
        min = vec3{numeric_limits<float>::max()};
        max = vec3{numeric_limits<float>::lowest()};
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
        // center = {(max + min) * 0.5f};
        // extents = { max.x - center.x, max.y - center.y, max.z - center.z};
    }

    // https://ktstephano.github.io/rendering/stratusgfx/aabbs
    AABB AABB::toGlobal(const mat4& transform) const {
        // First extract the 8 transformed corners of the box using vmin/vmax
        const glm::vec3 corners[8] = {
            glm::vec3(transform * glm::vec4(min.x, min.y, min.z, 1.0f)),
            glm::vec3(transform * glm::vec4(min.x, max.y, min.z, 1.0f)),
            glm::vec3(transform * glm::vec4(min.x, min.y, max.z, 1.0f)),
            glm::vec3(transform * glm::vec4(min.x, max.y, max.z, 1.0f)),
            glm::vec3(transform * glm::vec4(max.x, min.y, min.z, 1.0f)),
            glm::vec3(transform * glm::vec4(max.x, max.y, min.z, 1.0f)),
            glm::vec3(transform * glm::vec4(max.x, min.y, max.z, 1.0f)),
            glm::vec3(transform * glm::vec4(max.x, max.y, max.z, 1.0f))
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