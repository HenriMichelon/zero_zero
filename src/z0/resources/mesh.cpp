module;
/*
 * Derived from
 * https://github.com/blurrypiano/littleVulkanEngine
 * and
 * https://vulkan-tutorial.com/Loading_models
*/
#include <cassert>
#include "z0/libraries.h"
#include <volk.h>

module z0;

import :Material;
import :Buffer;
import :Application;
import :Tools;
import :Mesh;

namespace z0 {

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
        _buildModel();
    }

    void Mesh::setSurfaceMaterial(const uint32_t surfaceIndex, shared_ptr<Material> material) {
        surfaces[surfaceIndex]->material = std::move(material);
        _materials.insert(surfaces[surfaceIndex]->material);
    }

    vector<VkVertexInputBindingDescription2EXT> Mesh::_getBindingDescription() {
        vector<VkVertexInputBindingDescription2EXT> bindingDescriptions(1);
        bindingDescriptions[0].sType     = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
        bindingDescriptions[0].binding   = 0;
        bindingDescriptions[0].stride    = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescriptions[0].divisor   = 1;
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

    void Mesh::_draw(VkCommandBuffer commandBuffer, const uint32_t firstIndex, const uint32_t count) const {
        assert(vertexBuffer != nullptr && indexBuffer != nullptr);
        //////// Bind vertices & indices datas to command buffer
        VkBuffer     buffers[] = {vertexBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        //////// Draw mesh
        vkCmdDrawIndexed(commandBuffer, count, 1, firstIndex, 0, 0);
    }

    void Mesh::_buildModel() {
        const auto &device = Application::get()._getDevice();
        ////////////// Create vertices buffer
        const auto vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        constexpr auto vertexSize = sizeof(vertices[0]);
        const Buffer   vtxStagingBuffer{
                device,
                vertexSize,
                vertexCount,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT
        };
        vtxStagingBuffer.writeToBuffer(vertices.data());
        vertexBuffer = make_unique<Buffer>(
                device,
                vertexSize,
                vertexCount,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                );
        vtxStagingBuffer.copyTo(*vertexBuffer, sizeof (vertices[0]) * vertexCount);

        ////////////// Create indices buffer
        const auto indexCount = static_cast<uint32_t>(indices.size());
        if (indexCount <= 0)
            die("Unindexed meshes aren't supported");
        constexpr auto indexSize = sizeof(indices[0]);
        const Buffer   idxStagingBuffer{
                device,
                indexSize,
                indexCount,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        };
        idxStagingBuffer.writeToBuffer(indices.data());
        indexBuffer = make_unique<Buffer>(
                device,
                indexSize,
                indexCount,
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                );
        idxStagingBuffer.copyTo(*indexBuffer, sizeof (indices[0]) * indexCount);
    }

}
