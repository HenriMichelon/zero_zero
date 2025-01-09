/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>
#include "z0/libraries.h"

export module z0.vulkan.Mesh;

import z0.resources.Mesh;

import z0.vulkan.Buffer;

export namespace z0 {

    class VulkanMesh : public Mesh {
    public:
        inline explicit VulkanMesh(const string &meshName ) : Mesh{meshName} {}

        VulkanMesh(
            const vector<Vertex> &             vertices,
            const vector<uint32_t> &           indices,
            const vector<shared_ptr<Surface>> &surfaces,
            const string &                     meshName = "Mesh");

        VulkanMesh(VulkanMesh &&) = delete;
        VulkanMesh(VulkanMesh &) = delete;

        static vector<VkVertexInputBindingDescription2EXT> getBindingDescription();

        static vector<VkVertexInputAttributeDescription2EXT> getAttributeDescription();

        void bind(VkCommandBuffer commandBuffer) const;

        void buildModel();

    private:
        unique_ptr<Buffer> vertexBuffer;
        unique_ptr<Buffer> indexBuffer;
    };
}

