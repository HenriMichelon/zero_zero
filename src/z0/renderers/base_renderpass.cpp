#include "z0/renderers/base_renderpass.h"
#include "z0/resources/mesh.h"
#include "z0/descriptors.h"

#include <fstream>
#include <filesystem>

namespace z0 {

    BaseRenderpass::BaseRenderpass(const Device &dev, string sDir) :
            device{dev},
            vkDevice{dev.getDevice()},
            shaderDirectory(std::move(sDir)) {}

    void BaseRenderpass::cleanup() {
        globalUniformBuffers.clear();
        if (vertShader != nullptr) vertShader.reset();
        if (fragShader != nullptr) fragShader.reset();
        if (pipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(vkDevice, pipelineLayout, nullptr);
            pipelineLayout = VK_NULL_HANDLE;
        }
        setLayout.reset();
        descriptorPool.reset();
    }

    void BaseRenderpass::bindShaders(VkCommandBuffer commandBuffer) {
        if (vertShader != nullptr) {
            vkCmdBindShadersEXT(commandBuffer, 1, vertShader->getStage(), vertShader->getShader());
        } else {
            VkShaderStageFlagBits stageFlagBits{VK_SHADER_STAGE_VERTEX_BIT};
            vkCmdBindShadersEXT(commandBuffer, 1, &stageFlagBits, VK_NULL_HANDLE);
        }
        if (fragShader != nullptr) {
            vkCmdBindShadersEXT(commandBuffer, 1, fragShader->getStage(), fragShader->getShader());
        } else {
            VkShaderStageFlagBits stageFlagBits{VK_SHADER_STAGE_FRAGMENT_BIT};
            vkCmdBindShadersEXT(commandBuffer, 1, &stageFlagBits, VK_NULL_HANDLE);
        }
    }

    void BaseRenderpass::writeUniformBuffer(const std::vector<std::unique_ptr<Buffer>>& buffers, uint32_t currentFrame, void *data, uint32_t index) {
        uint32_t size = buffers[currentFrame]->getAlignmentSize();
        buffers[currentFrame]->writeToBuffer(data, size, size * index);
    }

    void BaseRenderpass::createUniformBuffers(std::vector<std::unique_ptr<Buffer>>& buffers, VkDeviceSize size, uint32_t count) {
        for (auto &uboBuffer: buffers) {
            uboBuffer = make_unique<Buffer>(
                    device,
                    size,
                    count,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    device.getDeviceProperties().limits.minUniformBufferOffsetAlignment
            );
            uboBuffer->map();
        }
    }

    void BaseRenderpass::bindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t currentFrame, uint32_t count, uint32_t *offsets) {
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipelineLayout,
                                0, 1,
                                &descriptorSets[currentFrame],
                                count, offsets);
    }

    void BaseRenderpass::createResources() {
        createDescriptorSetLayout();
        if (setLayout != nullptr) {
            createPipelineLayout();
            loadShaders();
        }
    };

    void BaseRenderpass::createPipelineLayout() {
        const VkPipelineLayoutCreateInfo pipelineLayoutInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .setLayoutCount = 1,
                .pSetLayouts = setLayout->getDescriptorSetLayout(),
                .pushConstantRangeCount = 0,
                .pPushConstantRanges = nullptr
        };
        if (vkCreatePipelineLayout(vkDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            die("failed to create pipeline layout!");
        }
    }

    std::unique_ptr<Shader> BaseRenderpass::createShader(const std::string& filename,
                                                               VkShaderStageFlagBits stage,
                                                               VkShaderStageFlags next_stage) {
        auto code = readFile(filename);
        std::unique_ptr<Shader> shader  = std::make_unique<Shader>(
                device,
                stage,
                next_stage,
                filename,
                code,
                setLayout->getDescriptorSetLayout(),
                nullptr);
        buildShader(*shader);
        return shader;
    }

    // https://docs.vulkan.org/samples/latest/samples/extensions/shader_object/README.html
    void BaseRenderpass::buildShader(Shader& shader) {
        VkShaderEXT shaderEXT;
        VkShaderCreateInfoEXT shaderCreateInfo = shader.getShaderCreateInfo();
        if (vkCreateShadersEXT(vkDevice, 1, &shaderCreateInfo, nullptr, &shaderEXT) != VK_SUCCESS) {
            die("vkCreateShadersEXT failed");
        }
        shader.setShader(shaderEXT);
    }

    void BaseRenderpass::setViewport(VkCommandBuffer commandBuffer, uint32_t width, uint32_t height) {
        const VkExtent2D extent = { width, height };
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

    std::vector<char> BaseRenderpass::readFile(const std::string &fileName) {
        std::filesystem::path filepath = shaderDirectory;
        filepath /= fileName;
        filepath += ".spv";
        std::ifstream file{filepath, std::ios::ate | std::ios::binary};
        if (!file.is_open()) {
            die("failed to open file : ", fileName);
        }
        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }

}