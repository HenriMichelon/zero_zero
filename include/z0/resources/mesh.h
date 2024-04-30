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

    struct MeshSurface {
        uint32_t firstVertexIndex;
        uint32_t indexCount;
        shared_ptr<Material> material;
        MeshSurface(uint32_t first, uint32_t count): firstVertexIndex{first}, indexCount{count} {};
    };

    class Mesh: public Resource {
    public:
        explicit Mesh(const std::string& meshName): Resource{meshName} {};

        vector<std::shared_ptr<MeshSurface>>& getSurfaces() { return surfaces; };
        shared_ptr<Material>& getSurfaceMaterial(uint32_t surfaceIndex);
        void setSurfaceMaterial(uint32_t surfaceIndex, shared_ptr<Material>& material);
        vector<Vertex>& getVertices() { return vertices; }
        vector<uint32_t>& getIndices() { return indices; }

    private:
        vector<Vertex> vertices{};
        vector<uint32_t> indices{};
        vector<std::shared_ptr<MeshSurface>> surfaces{};
        unordered_set<std::shared_ptr<Material>> _materials{};

        Device& device;
        uint32_t vertexCount{0};
        uint32_t indexCount{0};
        unique_ptr<Buffer> vertexBuffer;
        unique_ptr<Buffer> indexBuffer;

        void bind(VkCommandBuffer commandBuffer);
        void createVertexBuffers();
        void createIndexBuffers();

    public:
        unordered_set<std::shared_ptr<Material>>& _getMaterials() { return _materials; };
        static vector<VkVertexInputBindingDescription2EXT> _getBindingDescription();
        static vector<VkVertexInputAttributeDescription2EXT> _getAttributeDescription();
        void _draw(VkCommandBuffer commandBuffer, uint32_t first, uint32_t count);

        Mesh(const Mesh&) = delete;
        Mesh &operator=(const Mesh&) = delete;
    };

}