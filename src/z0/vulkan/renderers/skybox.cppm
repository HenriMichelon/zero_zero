/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/vulkan.h"
#include "z0/libraries.h"

export module z0.vulkan.SkyboxRenderer;

import z0.Constants;

import z0.nodes.Environment;
import z0.nodes.Camera;

import z0.resources.Cubemap;

import z0.vulkan.Descriptors;
import z0.vulkan.Buffer;
import z0.vulkan.Device;
import z0.vulkan.Renderpass;
import z0.vulkan.Cubemap;

export namespace z0 {

    class SkyboxRenderer : public Renderpass {
    public:
        SkyboxRenderer(Device &device, VkClearValue clearColor);

        void loadScene(const shared_ptr<Cubemap> &cubemap);

        void cleanup() override;

        void update(const shared_ptr<Camera>& currentCamera, const shared_ptr<Environment>& currentEnvironment, uint32_t currentFrame);

        void loadShaders() override;

        void createDescriptorSetLayout() override;

        void createOrUpdateDescriptorSet(bool create) override;

        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame);

        [[nodiscard]] inline auto getCubemap() const { return reinterpret_pointer_cast<Cubemap>(cubemap); }

    private:
        struct GobalUniformBuffer {
            mat4 projection{1.0f};
            mat4 view{1.0f};
            vec4 ambient{1.0f, 1.0f, 1.0f, 1.0f}; // RGB + Intensity;
        };

        vector<unique_ptr<Buffer>>  globalBuffer;
        uint32_t                    vertexCount;
        unique_ptr<Buffer>          vertexBuffer;
        shared_ptr<VulkanCubemap>   cubemap;
    };

}
