/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <cassert>
#include "z0/libraries.h"

module z0.Mesh;

import z0.Material;
import z0.Tools;

import z0.Device;
import z0.VulkanMesh;

namespace z0 {

    shared_ptr<Mesh> Mesh::create(const string &meshName) {
        return make_shared<VulkanMesh>(meshName);
    }

    shared_ptr<Mesh> Mesh::create(
            const vector<Vertex> &             vertices,
            const vector<uint32_t> &           indices,
            const vector<shared_ptr<Surface>> &surfaces,
            const string &                     meshName) {
        auto& device = Device::get();
        const auto commandPool = device.beginCommandPool();
        return make_shared<VulkanMesh>(commandPool, vertices, indices, surfaces, meshName);
        device.endCommandPool(commandPool);
    }


    Surface::Surface(const uint32_t firstIndex,
                     const uint32_t count):
        firstVertexIndex{firstIndex},
        indexCount{count},
        material{nullptr} {
    };

    Mesh::Mesh(const string &meshName):
        Resource{meshName} {
    }

    Mesh::Mesh(const vector<Vertex> &             vertices,
               const vector<uint32_t> &           indices,
               const vector<shared_ptr<Surface>> &surfaces,
               const string &                     meshName):
        Resource{meshName},
        vertices{vertices},
        indices{indices},
        surfaces{surfaces} {
        buildAABB();
    }

    void Mesh::setSurfaceMaterial(const uint32_t surfaceIndex, const shared_ptr<Material>& material) {
        assert(surfaceIndex < surfaces.size());
        surfaces[surfaceIndex]->material = material;
        materials.insert(surfaces[surfaceIndex]->material);
    }

    bool Mesh::operator==(const Mesh &other) const {
        return vertices == other.vertices &&
                indices == other.indices &&
                surfaces == other.surfaces &&
                materials == other.materials;
    }

     void Mesh::buildAABB() {
        auto min = vec3{numeric_limits<float>::max()};
        auto max = vec3{numeric_limits<float>::lowest()};
        for (const auto index : indices) {
            const auto& position = vertices[index].position;
            //Get the smallest vertex
            min.x = std::min(min.x, position.x);    // Find smallest x value in model
            min.y = std::min(min.y, position.y);    // Find smallest y value in model
            min.z = std::min(min.z, position.z);    // Find smallest z value in model
            //Get the largest vertex
            max.x = std::max(max.x, position.x);    // Find largest x value in model
            max.y = std::max(max.y, position.y);    // Find largest y value in model
            max.z = std::max(max.z, position.z);    // Find largest z value in model
        }
        localAABB = {min, max};
    }

}
