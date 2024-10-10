module;
#include "z0/libraries.h"
#include <volk.h>

module z0;

import :Tools;
import :Constants;
import :Device;
import :Shader;
import :Buffer;
import :Descriptors;
import :Renderpass;

namespace z0 {

    void Renderpass::cleanup() {
        globalUniformBuffers.clear();
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

    Renderpass::Renderpass(Device &dev, string shaderDir, const vec3 clearColor) :
        device{dev},
        vkDevice{dev.getDevice()},
        shaderDirectory(std::move(shaderDir)) {
        this->clearColor = {{{
                clearColor.r / 256.0f,
                clearColor.g / 256.0f,
                clearColor.b / 256.0f,
                1.0f}}};
    }

    Renderpass::Renderpass(Device &dev, string shaderDir, const VkClearValue clearColor) :
        device{dev},
        vkDevice{dev.getDevice()},
        shaderDirectory(std::move(shaderDir)),
        clearColor{clearColor} {
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

    void Renderpass::writeUniformBuffer(const vector<unique_ptr<Buffer>> &buffers,
                                        const uint32_t                    currentFrame,
                                        const void *                      data,
                                        const uint32_t                    index) {
        const auto size = buffers[currentFrame]->getAlignmentSize();
        buffers[currentFrame]->writeToBuffer(data, size, size * index);
    }

    void Renderpass::writeUniformBuffer(const vector<unique_ptr<Buffer>> &buffers,
                                        const uint32_t                    currentFrame,
                                        const void *                      data) {
        buffers[currentFrame]->writeToBuffer(data);
    }

    void Renderpass::createOrUpdateResources(const VkPushConstantRange* pushConstantRange) {
        if (pipelineLayout == VK_NULL_HANDLE && (pushConstantRange != nullptr)) {
            createPipelineLayout(pushConstantRange);
            loadShaders();
        } else if ((pipelineLayout == VK_NULL_HANDLE) && (descriptorPool == nullptr)) {
            descriptorSetNeedUpdate = false;
            createDescriptorSetLayout();
            createOrUpdateDescriptorSet(true);
            if (setLayout != nullptr) {
                createPipelineLayout();
                loadShaders();
            }
        } else if (descriptorSetNeedUpdate) {
            descriptorSetNeedUpdate = false;
            createOrUpdateDescriptorSet(false);
        }
    }

    void Renderpass::createUniformBuffers(vector<unique_ptr<Buffer>> &buffers,
                                          const VkDeviceSize          size,
                                          const uint32_t              count) const {
        for (auto &buffer : buffers) {
            buffer = make_unique<Buffer>(
                    device,
                    size,
                    count,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    device.getDeviceProperties().limits.minUniformBufferOffsetAlignment
                    );
            if (buffer->map() != VK_SUCCESS) { die("Error mapping UBO to GPU memory"); }
        }
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
        auto code   = readFile(filename);
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

    void Renderpass::createPipelineLayout(const VkPushConstantRange* pushConstantRange) {
        this->pushConstantRange = const_cast<VkPushConstantRange*>(pushConstantRange);
        const uint32_t pushConstantRangeCount = (pushConstantRange != nullptr ? 1 : 0);
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

    vector<char> Renderpass::readFile(const string &fileName) {
        filesystem::path filepath = shaderDirectory;
        filepath /= fileName;
        filepath += ".spv";
        ifstream file{filepath, std::ios::ate | std::ios::binary};
        if (!file.is_open()) {
            die("failed to open file : ", fileName);
        }
        const size_t fileSize = static_cast<size_t>(file.tellg());
        vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }

}
