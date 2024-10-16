module;
#include <cassert>
#include "z0/libraries.h"
#include <volk.h>

export module z0:Mesh;

import :Resource;
import :Buffer;
import :Material;
import :Tools;
import :AABB;

export namespace z0 {

    /**
     * A Mesh vertex
     */
    struct Vertex {
        //! local position
        vec3             position;
        //! surface normal
        vec3             normal;
        //! UV coordinates in the surface
        alignas(16) vec2 uv;
        //! UV based tangents
        vec4             tangent;

        inline bool operator==(const Vertex &other) const {
            return position == other.position && normal == other.normal && uv == other.uv && tangent == other.tangent;
        }
    };

    /**
     * A Mesh surface, with counterclockwise triangles
     */
    struct Surface {
        //! Index of the first vertex of the surface
        uint32_t             firstVertexIndex;
        //! Number of vertices
        uint32_t             indexCount;
        //! Material
        shared_ptr<Material> material;

        Surface( uint32_t firstIndex,
                 uint32_t count);

        inline bool operator==(const Surface &other) const {
            return firstVertexIndex == other.firstVertexIndex && indexCount == other.indexCount && material == other.material;
        }

        friend inline bool operator==(const shared_ptr<Surface>& a, const shared_ptr<Surface>& b) {
            return *a == *b;
        }
    };

    /**
     * A mesh composed by multiple Surface and an indexes collection of Vertex
     */
    class Mesh : public Resource {
    public:
        /**
         * Creates an empty Mesh
         * @param meshName node name
         */
        explicit Mesh(const string &meshName = "Mesh");

        /**
         * Creates a Mesh from vertices
         * @param vertices Vertices
         * @param indices Indexes of vertices
         * @param surfaces Surfaces
         * @param meshName Node name
         */
        Mesh(const vector<Vertex> &             vertices,
             const vector<uint32_t> &           indices,
             const vector<shared_ptr<Surface>> &surfaces,
             const string &                     meshName = "Mesh");

        /**
         * Returns the material for a given surface
         * @param surfaceIndex Zero based index of the surface
         */
        [[nodiscard]] inline const shared_ptr<Material> &getSurfaceMaterial(const uint32_t surfaceIndex) const {
            assert(surfaceIndex < surfaces.size());
            return surfaces[surfaceIndex]->material;
        }

        /**
         * Changes the material of a given surface
         * @param surfaceIndex Zero based index of the Surface
         * @param material New material for the Surface
         */
        void setSurfaceMaterial(uint32_t surfaceIndex, shared_ptr<Material> material);

        /**
         * Returns all the Surfaces
         */
        [[nodiscard]] inline vector<shared_ptr<Surface>> &getSurfaces() { return surfaces; }

        /**
         * Returns all the vertices
         */
        [[nodiscard]] inline vector<Vertex> &getVertices() { return vertices; }

        /**
         * Return all the vertices indexes
         */
        [[nodiscard]] inline vector<uint32_t> &getIndices() { return indices; }

        /**
         * Returns all the vertices
         */
        [[nodiscard]] inline const vector<Vertex> &getVertices() const { return vertices; }

        /**
         * Return all the vertices indexes
         */
        [[nodiscard]] inline const vector<uint32_t> &getIndices() const { return indices; }

        /**
         * Returns the local space axis aligned bounding box
         */
        [[nodiscard]] inline const AABB &getAABB() const { return localAABB; }

        bool operator==(const Mesh &other) const;

        friend inline bool operator==(const shared_ptr<Mesh>& a, const shared_ptr<Mesh>& b) {
            return (a == nullptr) ? false : *a == *b;
        }

        friend inline bool operator<(const shared_ptr<Mesh>& a, const shared_ptr<Mesh>& b) {
            return *a < *b;
        }

    private:
        AABB                                localAABB;
        vector<Vertex>                      vertices;
        vector<uint32_t>                    indices;
        vector<shared_ptr<Surface>>         surfaces{};
        unordered_set<shared_ptr<Material>> materials{};
        unique_ptr<Buffer>                  vertexBuffer;
        unique_ptr<Buffer>                  indexBuffer;

    public:
        [[nodiscard]] inline unordered_set<shared_ptr<Material>> &_getMaterials() { return materials; }

        [[nodiscard]] static vector<VkVertexInputBindingDescription2EXT> _getBindingDescription();

        [[nodiscard]] static vector<VkVertexInputAttributeDescription2EXT> _getAttributeDescription();

        // Bind vertices & indexes then draw the mesh
        void _draw(VkCommandBuffer commandBuffer, uint32_t firstIndex, uint32_t count) const;

        // Only send the drawing command without binding vertices & indexes
        void _bindlessDraw(VkCommandBuffer commandBuffer, uint32_t firstIndex, uint32_t count) const;

        // Build the vertices & index buffers and compute the local AABB
        void _buildModel();
    };
}

namespace std {
    /**
     * Custom hash function for Vertex struct (std lib code convention)
     */
    template <>
    struct hash<z0::Vertex> {
        std::size_t operator()(const z0::Vertex &v) const {
            std::size_t h1 = std::hash<vec3>()(v.position);
            std::size_t h2 = std::hash<vec3>()(v.normal);
            std::size_t h3 = std::hash<vec2>()(v.uv);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
}
