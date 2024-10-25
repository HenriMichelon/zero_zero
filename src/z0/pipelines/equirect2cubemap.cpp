module;
#include <volk.h>
#include "z0/libraries.h"

module z0;

import :Constants;
import :Tools;
import :Renderer;
import :Renderpass;
import :Device;
import :Image;
import :Cubemap;
import :Equirect2CubemapPipeline;
import :Descriptors;

namespace z0 {

    Equirect2CubemapPipeline::Equirect2CubemapPipeline(Device &device) :
        device{device} {
        descriptorPool =  DescriptorPool::Builder(device)
                            .setMaxSets(2)
                            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1) // HDRi input image
                            .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1) // Cubemap output image
                            .build();
        descriptorSetLayout = DescriptorSetLayout::Builder(device)
                   .addBinding(0,
                               VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                               VK_SHADER_STAGE_COMPUTE_BIT,
                               1)
                   .addBinding(1,
                               VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                               VK_SHADER_STAGE_COMPUTE_BIT,
                               1)
                   .build();
        pipelineLayout = createPipelineLayout(*descriptorSetLayout->getDescriptorSetLayout());
    }

    Equirect2CubemapPipeline::~Equirect2CubemapPipeline() {
        vkDestroyPipelineLayout(device.getDevice(), pipelineLayout, nullptr);
    }

    void Equirect2CubemapPipeline::convert(const shared_ptr<Image>&   hdrFile,
                                           const shared_ptr<Cubemap>& cubemap) const {
        const auto inputInfo = hdrFile->_getImageInfo();
        const auto outputInfo = VkDescriptorImageInfo{ VK_NULL_HANDLE, cubemap->getImageView(), VK_IMAGE_LAYOUT_GENERAL };
        auto descriptorSet = VkDescriptorSet{VK_NULL_HANDLE};
        if (!descriptorPool->allocateDescriptor(*descriptorSetLayout->getDescriptorSetLayout(), descriptorSet))
            die("Cannot allocate descriptor set");
        if (!DescriptorWriter(*descriptorSetLayout, *descriptorPool)
            .writeImage(0, &inputInfo)
            .writeImage(1, &outputInfo)
            .build(descriptorSet))
            die("Cannot allocate compute descriptor set");
        const auto shaderModule = createShaderModule(readFile("equirect2cube.comp"));
        const auto pipeline = createPipeline(pipelineLayout, shaderModule);

        const auto commandBuffer = device.beginOneTimeCommandBuffer();
        {
            const auto preComputeBarrier = array{VkImageMemoryBarrier {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask =  0,
                .dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_GENERAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = cubemap->getImage(),
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = VK_REMAINING_MIP_LEVELS,
                    .baseArrayLayer = 0,
                    .layerCount = VK_REMAINING_ARRAY_LAYERS,
                }
            }};
            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                0,
                0,
                nullptr,
                0,
                nullptr,
                static_cast<uint32_t>(preComputeBarrier.size()),
                preComputeBarrier.data());

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
            vkCmdDispatch(commandBuffer, cubemap->getWidth()/32, cubemap->getWidth()/32, 6);

            const auto posComputeBarrier = array{VkImageMemoryBarrier {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
                .dstAccessMask = 0,
                .oldLayout = VK_IMAGE_LAYOUT_GENERAL,
                .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = cubemap->getImage(),
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = VK_REMAINING_MIP_LEVELS,
                    .baseArrayLayer = 0,
                    .layerCount = VK_REMAINING_ARRAY_LAYERS,
                }
            }};
            vkCmdPipelineBarrier(commandBuffer,
                 VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                 VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                 0,
                 0,
                 nullptr,
                 0,
                 nullptr,
                 static_cast<uint32_t>(posComputeBarrier.size()),
                 posComputeBarrier.data());
        }
        device.endOneTimeCommandBuffer(commandBuffer);
        vkDestroyPipeline(device.getDevice(), pipeline, nullptr);
    }

    VkPipeline Equirect2CubemapPipeline::createPipeline(const VkPipelineLayout layout, const VkShaderModule shader) const {
        const auto shaderStage = VkPipelineShaderStageCreateInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VK_SHADER_STAGE_COMPUTE_BIT,
            .module = shader,
            .pName = "main",
            .pSpecializationInfo = nullptr,
        };
        const auto createInfo = VkComputePipelineCreateInfo {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .stage = shaderStage,
            .layout = layout,
        };
        auto pipeline = VkPipeline{VK_NULL_HANDLE};
        if(vkCreateComputePipelines(device.getDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline) != VK_SUCCESS)
            die("Failed to create compute pipeline");
        vkDestroyShaderModule(device.getDevice(), shader, nullptr);
        return pipeline;
    }

    VkPipelineLayout Equirect2CubemapPipeline::createPipelineLayout(VkDescriptorSetLayout descriptorSetLayout) const {
        const auto pipelineSetLayouts = array{ descriptorSetLayout };
        const auto createInfo = VkPipelineLayoutCreateInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = pipelineSetLayouts.size(),
            .pSetLayouts = pipelineSetLayouts.data(),
        };
        auto computePipelineLayout = VkPipelineLayout{VK_NULL_HANDLE};
        if(vkCreatePipelineLayout(device.getDevice(), &createInfo, nullptr, &computePipelineLayout) != VK_SUCCESS) {
            die("Failed to create pipeline layout");
        }
        return  computePipelineLayout;
    }

    VkShaderModule Equirect2CubemapPipeline::createShaderModule(const vector<char>& code) const {
        const auto createInfo = VkShaderModuleCreateInfo {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = code.size(),
            .pCode    = reinterpret_cast<const uint32_t*>(&code[0])
        };
        auto shaderModule = VkShaderModule{VK_NULL_HANDLE};
        if(vkCreateShaderModule(device.getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
            die("Failed to create shader module");
        return shaderModule;
    }

    vector<char> Equirect2CubemapPipeline::readFile(const string &fileName) const {
        filesystem::path filepath = (Application::get().getConfig().appDir / "shaders").string();
        filepath /= fileName;
        filepath += ".spv";
        ifstream file{filepath, std::ios::ate | std::ios::binary};
        if (!file.is_open()) {
            die("failed to open file : ", filepath.string());
        }
        const size_t fileSize = file.tellg();
        vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }
} // namespace z0
