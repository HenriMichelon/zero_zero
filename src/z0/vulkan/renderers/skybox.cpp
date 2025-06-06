/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/vulkan.h"
#include "z0/libraries.h"

module z0.vulkan.SkyboxRenderer;

import z0.Tools;

import z0.nodes.Environment;
import z0.nodes.Camera;

import z0.resources.Cubemap;

import z0.vulkan.Buffer;
import z0.vulkan.Descriptors;
import z0.vulkan.Device;
import z0.vulkan.Cubemap;

namespace z0 {

    SkyboxRenderer::SkyboxRenderer(Device &device, const VkClearValue clearColor):
        Renderpass{device, clearColor} {
        globalBuffer.resize(device.getFramesInFlight());
        static constexpr float skyboxVertices[] = {
                // positions
            -1.0f, 1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            -1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f
        };
        vertexCount                   = 108 / 3;
        uint32_t           vertexSize{sizeof(float) * 3};
        const VkDeviceSize bufferSize{vertexSize * vertexCount};

        const auto command = device.beginOneTimeCommandBuffer();
        const auto& stagingBuffer = device.createOneTimeBuffer(
                command,
                vertexSize,
                vertexCount,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT
        );
        stagingBuffer.writeToBuffer(skyboxVertices);
        vertexBuffer = make_unique<Buffer>(
                vertexSize,
                vertexCount,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                );
        stagingBuffer.copyTo(command.commandBuffer, *vertexBuffer, bufferSize);
        device.endOneTimeCommandBuffer(command);
    }

    void SkyboxRenderer::loadScene(const shared_ptr<Cubemap> &cubemap) {
        this->cubemap = reinterpret_pointer_cast<VulkanCubemap>(cubemap);
        createOrUpdateResources();
    }

    void SkyboxRenderer::cleanup() {
        globalBuffer.clear();
        vertexBuffer.reset();
        cubemap.reset();
        Renderpass::cleanup();
    }

    void SkyboxRenderer::update(const shared_ptr<Camera>&      currentCamera,
                                const shared_ptr<Environment>& currentEnvironment,
                                const uint32_t      currentFrame) {
        GobalUniformBuffer globalUbo{
                .projection = currentCamera->getProjection(),
                .view = mat4(mat3(currentCamera->getView()))
        };
        if (currentEnvironment != nullptr) {
            globalUbo.ambient = currentEnvironment->getAmbientColorAndIntensity();
        }
        writeUniformBuffer(globalBuffer.at(currentFrame), &globalUbo);
    }

    void SkyboxRenderer::loadShaders() {
        vertShader = createShader("skybox.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
        fragShader = createShader("skybox.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void SkyboxRenderer::createDescriptorSetLayout() {
        descriptorPool = DescriptorPool::Builder(device)
                         .setMaxSets(device.getFramesInFlight())
                         .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, device.getFramesInFlight())
                         .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, device.getFramesInFlight())
                         .build();

        for (auto i = 0; i < device.getFramesInFlight(); i++) {
            globalBuffer.at(i) = createUniformBuffer(sizeof(GobalUniformBuffer));
        }

        setLayout = DescriptorSetLayout::Builder(device)
                    .addBinding(0,
                                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                VK_SHADER_STAGE_VERTEX_BIT)
                    .addBinding(1,
                                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                VK_SHADER_STAGE_FRAGMENT_BIT,
                                1)
                    .build();
    }

    void SkyboxRenderer::createOrUpdateDescriptorSet(const bool create) {
        if (create) {
            shared_ptr<VulkanCubemap> cm;
            if (cubemap->getCubemapType() == Cubemap::TYPE_ENVIRONMENT) {
                const auto& environment = reinterpret_pointer_cast<EnvironmentCubemap>(cubemap);
                cm = reinterpret_pointer_cast<VulkanCubemap>(environment->getSpecularCubemap());
            } else {
                cm = cubemap;
            }
            for (auto i = 0; i < device.getFramesInFlight(); i++) {
                auto globalBufferInfo = globalBuffer.at(i)->descriptorInfo(sizeof(GobalUniformBuffer));
                auto imageInfo        = cm->getImageInfo();
                auto writer           = DescriptorWriter(*setLayout, *descriptorPool)
                    .writeBuffer(0, &globalBufferInfo)
                    .writeImage(1, &imageInfo);
                if (!writer.build(descriptorSet.at(i), create))
                    die("Cannot allocate skybox renderer descriptor set");
            }
        }
    }

    void SkyboxRenderer::recordCommands(const VkCommandBuffer commandBuffer, const uint32_t currentFrame) {
        bindShaders(commandBuffer);
        vkCmdSetDepthTestEnable(commandBuffer, VK_TRUE);
        vkCmdSetDepthWriteEnable(commandBuffer, VK_FALSE);
        constexpr VkVertexInputBindingDescription2EXT bindingDescription{
                .sType     = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,
                .binding   = 0,
                .stride    = 3 * sizeof(float),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
                .divisor   = 1,
        };
        constexpr VkVertexInputAttributeDescription2EXT attributeDescription{
                VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
                nullptr,
                0,
                0,
                VK_FORMAT_R32G32B32_SFLOAT,
                0
        };
        vkCmdSetVertexInputEXT(commandBuffer,
                               1,
                               &bindingDescription,
                               1,
                               &attributeDescription);
        vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_BACK_BIT);
        vkCmdSetDepthCompareOp(commandBuffer, VK_COMPARE_OP_LESS_OR_EQUAL);
        bindDescriptorSets(commandBuffer, currentFrame);
        const VkBuffer         buffers[] = {vertexBuffer->getBuffer()};
        constexpr VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
    }

}
