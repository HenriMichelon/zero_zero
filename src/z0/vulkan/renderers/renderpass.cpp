/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/vulkan.h"
#include "z0/libraries.h"

module z0.vulkan.Renderpass;

import z0.Tools;
import z0.Constants;
import z0.VirtualFS;

import z0.vulkan.Buffer;
import z0.vulkan.Device;
import z0.vulkan.Descriptors;
import z0.vulkan.Shader;

namespace z0 {

    void Renderpass::cleanup() {
        if (vertShader != nullptr)
            vertShader.reset();
        if (fragShader != nullptr)
            fragShader.reset();
        if (pipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(vkDevice, pipelineLayout, nullptr);
            pipelineLayout = VK_NULL_HANDLE;
        }
        setLayout.reset();
        descriptorPool.reset();
    }

    Renderpass::Renderpass(Device &dev, const vec3 clearColor) :
        Renderpass(dev, {
            clearColor.r,
            clearColor.g,
            clearColor.b,
            1.0f}) {
    }

    Renderpass::Renderpass(Device &dev, const VkClearValue clearColor) :
        device{dev},
        vkDevice{dev.getDevice()},
        clearColor{clearColor} {
        descriptorSet.resize(dev.getFramesInFlight());
    }

    void Renderpass::setViewport(const VkCommandBuffer commandBuffer,
                                 const uint32_t        width,
                                 const uint32_t        height) {
        const VkExtent2D extent = {width, height};
        const VkViewport viewport{
                .x = 0.0f,
                .y = 0.0f,
                .width = static_cast<float>(extent.width),
                .height = static_cast<float>(extent.height),
                .minDepth = 0.0f,
                .maxDepth = 1.0f
        };
        vkCmdSetViewportWithCount(commandBuffer, 1, &viewport);
        const VkRect2D scissor{
                .offset = {0, 0},
                .extent = extent
        };
        vkCmdSetScissorWithCount(commandBuffer, 1, &scissor);
    }

    void Renderpass::writeUniformBuffer(const unique_ptr<Buffer> &buffer,
                                        const void *                      data,
                                        const uint32_t                    index) {
        const auto size = buffer->getAlignmentSize();
        buffer->writeToBuffer(data, size, size * index);
    }

    void Renderpass::writeUniformBuffer(const unique_ptr<Buffer> &buffer,
                                        const void *                      data) {
        buffer->writeToBuffer(data);
    }

    void Renderpass::createOrUpdateResources(const bool descriptorsAndPushConstants, const VkPushConstantRange* pushConstantRange, const uint32_t pushConstantSize) {
        if (!descriptorsAndPushConstants && pipelineLayout == VK_NULL_HANDLE && (pushConstantRange != nullptr)) {
            createPipelineLayout(pushConstantRange, pushConstantSize);
            loadShaders();
        } else if ((pipelineLayout == VK_NULL_HANDLE) && (descriptorPool == nullptr)) {
            descriptorSetNeedUpdate = false;
            createDescriptorSetLayout();
            createOrUpdateDescriptorSet(true);
            if (setLayout != nullptr) {
                createPipelineLayout(pushConstantRange, pushConstantSize);
                loadShaders();
            }
        } else if (descriptorSetNeedUpdate) {
            descriptorSetNeedUpdate = false;
            createOrUpdateDescriptorSet(false);
        }
    }

    unique_ptr<Buffer> Renderpass::createUniformBuffer(const VkDeviceSize size,
                                                       const uint32_t     count) const {
        auto buffer = make_unique<Buffer>(
                size,
                count,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                device.getDeviceProperties().limits.minUniformBufferOffsetAlignment
                );
        if (buffer->map() != VK_SUCCESS) { die("Error mapping UBO to GPU memory"); }
        return std::move(buffer);
    }

    void Renderpass::bindDescriptorSets(const VkCommandBuffer commandBuffer,
                                        const uint32_t        currentFrame,
                                        const uint32_t        count,
                                        const uint32_t *      offsets) const {
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipelineLayout,
                                0,
                                1,
                                &descriptorSet[currentFrame],
                                count,
                                offsets);
    }

    void Renderpass::bindShaders(const VkCommandBuffer commandBuffer) const {
        if (vertShader != nullptr) {
            vkCmdBindShadersEXT(commandBuffer, 1, vertShader->getStage(), vertShader->getShader());
        } else {
            constexpr VkShaderStageFlagBits stageFlagBits{VK_SHADER_STAGE_VERTEX_BIT};
            vkCmdBindShadersEXT(commandBuffer, 1, &stageFlagBits, VK_NULL_HANDLE);
        }
        if (fragShader != nullptr) {
            vkCmdBindShadersEXT(commandBuffer, 1, fragShader->getStage(), fragShader->getShader());
        } else {
            constexpr VkShaderStageFlagBits stageFlagBits{VK_SHADER_STAGE_FRAGMENT_BIT};
            vkCmdBindShadersEXT(commandBuffer, 1, &stageFlagBits, VK_NULL_HANDLE);
        }
    }

    unique_ptr<Shader> Renderpass::createShader(const string &              filename,
                                                const VkShaderStageFlagBits stage,
                                                const VkShaderStageFlags    next_stage) {
        auto code = VirtualFS::loadShader(filename);
        auto shader = make_unique<Shader>(
                device,
                stage,
                next_stage,
                filename,
                code,
                setLayout != nullptr && setLayout->isValid() ? setLayout->getDescriptorSetLayout() : nullptr,
                pushConstantRange);
        buildShader(*shader);
        return shader;
    }

    void Renderpass::createPipelineLayout(const VkPushConstantRange* pushConstantRange, const uint32_t pushConstantSize) {
        this->pushConstantRange = const_cast<VkPushConstantRange*>(pushConstantRange);
        const uint32_t pushConstantRangeCount = (pushConstantRange != nullptr ? pushConstantSize : 0);
        const uint32_t setLayoutCount = (setLayout != nullptr && setLayout->isValid() ? 1 : 0);
        const VkPipelineLayoutCreateInfo pipelineLayoutInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .setLayoutCount = setLayoutCount,
                .pSetLayouts = setLayoutCount > 0 ? setLayout->getDescriptorSetLayout() : nullptr,
                .pushConstantRangeCount = pushConstantRangeCount,
                .pPushConstantRanges = pushConstantRange
        };
        if (vkCreatePipelineLayout(vkDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            die("failed to create pipeline layout!");
        }
    }

    void Renderpass::buildShader(Shader &shader) const {
        // https://docs.vulkan.org/samples/latest/samples/extensions/shader_object/README.html
        VkShaderEXT                 shaderEXT;
        const VkShaderCreateInfoEXT shaderCreateInfo = shader.getShaderCreateInfo();
        if (vkCreateShadersEXT(vkDevice, 1, &shaderCreateInfo, nullptr, &shaderEXT) != VK_SUCCESS) {
            die("vkCreateShadersEXT failed");
        }
        shader.setShader(shaderEXT);
    }

}
