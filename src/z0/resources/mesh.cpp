/*
 * Derived from
 * https://github.com/blurrypiano/littleVulkanEngine
 * and
 * https://vulkan-tutorial.com/Loading_models
*/

#include "z0/resources/mesh.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <cassert>
#include <unordered_map>

namespace  z0 {
    // from: https://stackoverflow.com/a/57595105
    template<typename T, typename... Rest>
    void hashCombine(std::size_t &seed, const T &v, const Rest &... rest) {
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        (hashCombine(seed, rest), ...);
    };
}

namespace std {
    template<>
    struct hash<z0::Vertex>{
        size_t operator()(z0::Vertex const &vertex) const {
            size_t seed = 0;
            z0::hashCombine(seed, vertex.position, vertex.normal, vertex.uv);
            return seed;
        }
    };
}

namespace z0 {

    const shared_ptr<Material>& Mesh::getSurfaceMaterial(uint32_t surfaceIndex) const {
        return surfaces[surfaceIndex]->material;
    }

    void Mesh::setSurfaceMaterial(uint32_t surfaceIndex, shared_ptr<Material>& material) {
        surfaces[surfaceIndex]->material = material;
    }

    Mesh::Mesh(const Device &dev, const vector<Vertex> &vtx, const vector<uint32_t> &idx, const std::string& meshName):
            Resource{meshName},
            device{dev},
            vertices{vtx},
            indices(idx){
        ////////////// Create vertices buffer
        const auto vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at leat 3");
        const auto vertexSize = sizeof(vertices[0]);
        const Buffer vtxStagingBuffer {
                device,
                vertexSize,
                vertexCount,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };
        vtxStagingBuffer.writeToBuffer((void*)vertices.data());
        vertexBuffer = make_unique<Buffer>(
                device,
                vertexSize,
                vertexCount,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
        vtxStagingBuffer.copyTo(*vertexBuffer, sizeof (vertices[0]) * vertexCount);

        ////////////// Create indicex buffer
        const auto indexCount = static_cast<uint32_t>(indices.size());
        if (indexCount <= 0) die("Unindexed meshes aren't supported");
        const auto indexSize = sizeof(indices[0]);
        const Buffer idxStagingBuffer {
                device,
                indexSize,
                indexCount,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        };
        idxStagingBuffer.writeToBuffer((void*)indices.data());
        indexBuffer = make_unique<Buffer>(
                device,
                indexSize,
                indexCount,
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
        idxStagingBuffer.copyTo(*indexBuffer, sizeof (indices[0]) * indexCount);
    }

    void Mesh::_draw(VkCommandBuffer commandBuffer, uint32_t firstIndex, uint32_t count)  const{
        //////// Bind to command buffer
        VkBuffer buffers[] = { vertexBuffer->getBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        //////// Draw mesh
        vkCmdDrawIndexed(commandBuffer, count, 1, firstIndex, 0, 0);
    }

    vector<VkVertexInputBindingDescription2EXT> Mesh::_getBindingDescription() {
        vector<VkVertexInputBindingDescription2EXT> bindingDescriptions(1);
        bindingDescriptions[0].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescriptions[0].divisor = 1;
        return bindingDescriptions;
    }

    vector<VkVertexInputAttributeDescription2EXT> Mesh::_getAttributeDescription() {
        vector<VkVertexInputAttributeDescription2EXT> attributeDescriptions{};
        attributeDescriptions.push_back({
                                                VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
                                                nullptr,
                                                0,
                                                0,
                                                VK_FORMAT_R32G32B32_SFLOAT,
                                                offsetof(Vertex, position)
                                        });
        attributeDescriptions.push_back({
                                                VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
                                                nullptr,
                                                1,
                                                0,
                                                VK_FORMAT_R32G32B32_SFLOAT,
                                                offsetof(Vertex, normal)
                                        });
        attributeDescriptions.push_back({
                                                VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
                                                nullptr,
                                                2,
                                                0,
                                                VK_FORMAT_R32G32_SFLOAT,
                                                offsetof(Vertex, uv)
                                        });
        attributeDescriptions.push_back({
                                                VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
                                                nullptr,
                                                3,
                                                0,
                                                VK_FORMAT_R32G32B32A32_SFLOAT,
                                                offsetof(Vertex, tangent)
                                        });
        return attributeDescriptions;
    }

}