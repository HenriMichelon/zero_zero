/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>
#include "z0/libraries.h"

export module z0.VulkanMesh;

import z0.Mesh;

import z0.Buffer;

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

        // Bind vertices & indexes then draw the mesh
        void draw(VkCommandBuffer commandBuffer, uint32_t firstIndex, uint32_t count) const;

        // Only send the drawing command without binding vertices & indexes
        void bindlessDraw(VkCommandBuffer commandBuffer, uint32_t firstIndex, uint32_t count) const;

        void buildModel();

    private:
        unique_ptr<Buffer> vertexBuffer;
        unique_ptr<Buffer> indexBuffer;
    };
}

