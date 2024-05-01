#pragma once

#include "z0/resources/material.h"
#include "z0/buffer.h"

#include <unordered_set>
#include <vector>

namespace z0 {

    struct Vertex {
        vec3   position{};
        vec3   normal{};
        alignas(16) vec2 uv{};
        vec4   tangent{};

        bool operator==(const Vertex&other) const {
            return position == other.position && normal == other.normal && uv == other.uv;
        }
    };

    struct Surface {
        uint32_t firstVertexIndex;
        uint32_t indexCount;
        shared_ptr<Material> material;
        Surface(uint32_t first, uint32_t count): firstVertexIndex{first}, indexCount{count} {};
    };

    class Mesh: public Resource {
    public:
        Mesh(const Device &dev, const vector<Vertex> &vertices, const vector<uint32_t> &indices, const std::string& meshName);

        const vector<std::shared_ptr<Surface>>& getSurfaces() const { return surfaces; };
        const shared_ptr<Material>& getSurfaceMaterial(uint32_t surfaceIndex) const;
        void setSurfaceMaterial(uint32_t surfaceIndex, shared_ptr<Material>& material);
        vector<Vertex>& getVertices() { return vertices; }
        vector<uint32_t>& getIndices() { return indices; }

    private:
        const Device& device;
        vector<Vertex> vertices{};
        vector<uint32_t> indices{};
        vector<std::shared_ptr<Surface>> surfaces{};
        unordered_set<std::shared_ptr<Material>> _materials{};
        unique_ptr<Buffer> vertexBuffer;
        unique_ptr<Buffer> indexBuffer;

    public:
        const unordered_set<std::shared_ptr<Material>>& _getMaterials() const { return _materials; };
        static vector<VkVertexInputBindingDescription2EXT> _getBindingDescription();
        static vector<VkVertexInputAttributeDescription2EXT> _getAttributeDescription();
        void _draw(VkCommandBuffer commandBuffer, uint32_t first, uint32_t count) const;

        Mesh(const Mesh&) = delete;
        Mesh &operator=(const Mesh&) = delete;
    };

}