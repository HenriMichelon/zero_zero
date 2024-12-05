/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>
#include "z0/libraries.h"

export module z0.SkyboxRenderer;

import z0.Constants;

import z0.nodes.Environment;
import z0.nodes.Camera;

import z0.resources.Cubemap;

import z0.Device;
import z0.Buffer;
import z0.Descriptors;
import z0.Renderpass;
import z0.VulkanCubemap;

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

        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

        [[nodiscard]] inline shared_ptr<Cubemap> getCubemap() const { return reinterpret_pointer_cast<Cubemap>(cubemap); }

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
