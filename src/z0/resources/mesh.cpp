/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <cassert>
namespace meshopt {
#include <meshoptimizer.h>
}
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
        return make_shared<VulkanMesh>(vertices, indices, surfaces, meshName);
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

    void Mesh::optimize() {
        vector<uint32_t> remap(indices.size()); // allocate temporary memory for the remap table
        auto vertex_count = meshopt::meshopt_generateVertexRemap(
            &remap[0],
            indices.data(),
            indices.size(),
            vertices.data(),
            vertices.size(),
            sizeof(Vertex));
        auto optIndices = vector<uint32_t>(indices.size());
        auto optVertices = vector<Vertex>(vertex_count);
        meshopt::meshopt_remapIndexBuffer(
            optIndices.data(),
            indices.data(),
            indices.size(),
            remap.data());
        meshopt::meshopt_remapVertexBuffer(
            optVertices.data(),
            vertices.data(),
            vertices.size(),
            sizeof(Vertex),
            remap.data());
        // cout << vertices.size() << " -> " << optVertices.size() << endl;
        // indices = optIndices;
        // vertices = optVertices;

        // Step 1: Optimize for vertex cache
        meshopt::meshopt_optimizeVertexCache(
            indices.data(),
            optIndices.data(),
            optIndices.size(),
            optVertices.size());

        // Step 2: Optimize for overdraw
        meshopt::meshopt_optimizeOverdraw(
            optIndices.data(),
            indices.data(),
            indices.size(),
            &optVertices[0].position.x,
            optVertices.size(),
            sizeof(Vertex),
            1.05f);

        // Step 3: Optimize vertex fetch
        meshopt::meshopt_optimizeVertexFetch(
            vertices.data(),
            indices.data(),
            indices.size(),
            optVertices.data(),
            optVertices.size(),
            sizeof(Vertex));

        float threshold = 0.2f;
        size_t target_index_count = static_cast<size_t>(indices.size() * threshold);
        float target_error = 1e-2f;

        vector<uint32_t> lod(indices.size());
        float lod_error = 0.f;
        lod.resize(meshopt::meshopt_simplify(
            &lod[0],
            indices.data(),
            indices.size(),
            &vertices[0].position.x,
            vertices.size(),
            sizeof(Vertex),
            target_index_count,
            target_error,
            /* options= */ 0,
            &lod_error));
        // indices = lod;
        log(to_string(optIndices.size()), " -> ", to_string(indices.size()));
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
