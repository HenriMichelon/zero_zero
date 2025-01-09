/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
/*
 * Derived from
 * https://github.com/blurrypiano/littleVulkanEngine
 * and
 * https://vulkan-tutorial.com/Loading_models
*/
#include <cassert>
#include <volk.h>
#include "z0/libraries.h"

module z0.vulkan.Mesh;

import z0.Tools;

import z0.resources.Material;

import z0.vulkan.Buffer;
import z0.vulkan.Device;

namespace z0 {

    VulkanMesh::VulkanMesh(
            const vector<Vertex> &             vertices,
            const vector<uint32_t> &           indices,
            const vector<shared_ptr<Surface>> &surfaces,
            const string &                      meshName) :
            Mesh(vertices, indices, surfaces, meshName) {
        buildModel();
    };

    vector<VkVertexInputBindingDescription2EXT> VulkanMesh::getBindingDescription() {
        vector<VkVertexInputBindingDescription2EXT> bindingDescriptions(1);
        bindingDescriptions[0].sType     = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
        bindingDescriptions[0].binding   = 0;
        bindingDescriptions[0].stride    = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescriptions[0].divisor   = 1;
        return bindingDescriptions;
    }

    vector<VkVertexInputAttributeDescription2EXT> VulkanMesh::getAttributeDescription() {
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

    void VulkanMesh::bind(const VkCommandBuffer commandBuffer) const {
        assert(vertexBuffer != nullptr && indexBuffer != nullptr);
        //////// Bind vertices & indices datas to command buffer
        const VkBuffer         buffers[] = {vertexBuffer->getBuffer()};
        constexpr VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }

    void VulkanMesh::buildModel() {
        optimize();
        const auto &device = Device::get();
        ////////////// Create vertices buffer
        const auto vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        constexpr auto vertexSize = sizeof(vertices[0]);
        const auto command = device.beginOneTimeCommandBuffer();
        const auto& vtxStagingBuffer = device.createOneTimeBuffer(
                command,
                vertexSize,
                vertexCount,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT
        );
        vtxStagingBuffer.writeToBuffer(vertices.data());
        vertexBuffer = make_unique<Buffer>(
                vertexSize,
                vertexCount,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                );
        vtxStagingBuffer.copyTo(command.commandBuffer, *vertexBuffer, sizeof (vertices[0]) * vertexCount);

        ////////////// Create indices buffer
        const auto indexCount = static_cast<uint32_t>(indices.size());
        if (indexCount <= 0) {
            die("Unindexed meshes aren't supported");
        }
        constexpr auto indexSize = sizeof(indices[0]);
        const auto&  idxStagingBuffer = device.createOneTimeBuffer(
                command,
                indexSize,
                indexCount,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT
        );
        idxStagingBuffer.writeToBuffer(indices.data());
        indexBuffer = make_unique<Buffer>(
                indexSize,
                indexCount,
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                );
        idxStagingBuffer.copyTo(command.commandBuffer, *indexBuffer, sizeof (indices[0]) * indexCount);
        device.endOneTimeCommandBuffer(command);

        buildAABB();
    }


}
