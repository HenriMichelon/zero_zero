module;
#include "z0/libraries.h"
#include <volk.h>

module z0;

import :Tools;
import :Renderpass;
import :Device;
import :Cubemap;
import :Environment;
import :Camera;
import :Buffer;
import :Descriptors;
import :SkyboxRenderer;

namespace z0 {

    SkyboxRenderer::SkyboxRenderer(Device &device, const string &shaderDirectory, const VkClearValue clearColor):
        Renderpass{device, shaderDirectory, clearColor} {
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
        const Buffer       stagingBuffer{
                device,
                vertexSize,
                vertexCount,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT
        };
        stagingBuffer.writeToBuffer(skyboxVertices);
        vertexBuffer = make_unique<Buffer>(
                device,
                vertexSize,
                vertexCount,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                );
        stagingBuffer.copyTo(*vertexBuffer, bufferSize);
    }

    void SkyboxRenderer::loadScene(const shared_ptr<Cubemap> &cubemap) {
        this->cubemap = cubemap;
        createOrUpdateResources();
    }

    void SkyboxRenderer::cleanup() {
        globalBuffer.clear();
        vertexBuffer.reset();
        cubemap.reset();
        Renderpass::cleanup();
    }

    void SkyboxRenderer::update(const Camera *      currentCamera,
                                const Environment * currentEnvironment,
                                const uint32_t      currentFrame) {
        GobalUniformBuffer globalUbo{
                .projection = currentCamera->getProjection(),
                .view = mat4(mat3(currentCamera->getView()))
        };
        if (currentEnvironment != nullptr) {
            globalUbo.ambient = currentEnvironment->getAmbientColorAndIntensity();
        }
        writeUniformBuffer(globalBuffer[currentFrame], &globalUbo);
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
            globalBuffer[i] = createUniformBuffer(sizeof(GobalUniformBuffer));
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
            for (auto i = 0; i < device.getFramesInFlight(); i++) {
                auto globalBufferInfo = globalBuffer[i]->descriptorInfo(sizeof(GobalUniformBuffer));
                auto imageInfo        = cubemap->_getImageInfo();
                auto writer           = DescriptorWriter(*setLayout, *descriptorPool)
                                        .writeBuffer(0, &globalBufferInfo)
                                        .writeImage(1, &imageInfo);
                if (!writer.build(descriptorSet[i]))
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
