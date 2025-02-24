/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <cassert>
#include "z0/libraries.h"
#ifndef _MSC_VER
namespace meshopt {
#endif
#include <meshoptimizer.h>
#ifndef _MSC_VER
}
using namespace meshopt;
#endif

module z0.resources.Mesh;

import z0.Application;
import z0.Log;

import z0.resources.Material;

import z0.vulkan.Device;
import z0.vulkan.Mesh;

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
    }

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
        vector<uint32_t> remap(indices.size());
        const auto newVertexCount = meshopt_generateVertexRemap(
            remap.data(),
            indices.data(),
            indices.size(),
            vertices.data(),
            vertices.size(),
            sizeof(Vertex));

        meshopt_remapIndexBuffer(
           indices.data(),
           indices.data(),
           indices.size(),
           remap.data());

        auto remappedVertices = vector<Vertex>(newVertexCount);
        meshopt_remapVertexBuffer(
            remappedVertices.data(),
            vertices.data(),
            vertices.size(),
            sizeof(Vertex),
            remap.data());

        DEBUG("Mesh::optimize ", getName(), ", vertices : ", vertices.size(), " -> ", remappedVertices.size());
        vertices = std::move(remappedVertices);

        // https://github.com/zeux/meshoptimizer/issues/624
        for (const auto& surface : surfaces) {
            meshopt_optimizeVertexCache(
                &indices[surface->firstVertexIndex],
                &indices[surface->firstVertexIndex],
                surface->indexCount,
                vertices.size());

            meshopt_optimizeOverdraw(
                &indices[surface->firstVertexIndex],
                &indices[surface->firstVertexIndex],
                surface->indexCount,
                &vertices[0].position.x,
                vertices.size(),
                sizeof(Vertex),
                1.05f);

        }
        meshopt_optimizeVertexFetch(
                vertices.data(),
                indices.data(),
                indices.size(),
                vertices.data(),
                vertices.size(),
                sizeof(Vertex));

        const auto threshold = app().getConfig().meshSimplifyThreshold;
        if (surfaces.size() == 1 && threshold < 1.0f) {
            constexpr float  target_error = 1e-2f;
            float            lod_error    = 0.f;
            const size_t     target_index_count = threshold == 0.0f ? 0 : static_cast<size_t>(indices.size() * threshold);
            vector<uint32_t> lod(indices.size());
            lod.resize(meshopt_simplify(
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
            DEBUG("Mesh::optimize simplify ", getName(), " : ", indices.size(), " -> ", lod.size());
            indices = lod;
            surfaces[0]->indexCount = indices.size();
        }
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
