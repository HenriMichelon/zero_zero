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

    void Equirect2CubemapPipeline::convert(const shared_ptr<Image>&   hdrFile,
                                           const shared_ptr<Cubemap>& cubemap) const {
        const auto hdrImageInfo = hdrFile->_getImageInfo();
        const auto cubemapInfo = cubemap->_getImageInfo();
        auto spirv = readFile("equirect2cube.comp");

        const auto computeDescriptorPool =  DescriptorPool::Builder(device)
                                    .setMaxSets(device.getFramesInFlight())
                                    .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1) // HDRi input image
                                    .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, device.getFramesInFlight()) // Cubemap output image
                                    .build();

        const auto descriptorSetLayout = DescriptorSetLayout::Builder(device)
                   .addBinding(0,
                               VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                               VK_SHADER_STAGE_COMPUTE_BIT,
                               1)
                   .addBinding(1,
                               VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                               VK_SHADER_STAGE_COMPUTE_BIT,
                               1)
                   .build();

        auto computePipelineLayout = VkPipelineLayout{VK_NULL_HANDLE};
        {
            const auto pipelineSetLayouts = array<VkDescriptorSetLayout, 1>{ *descriptorSetLayout->getDescriptorSetLayout() };
            const auto createInfo = VkPipelineLayoutCreateInfo {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .setLayoutCount = pipelineSetLayouts.size(),
                .pSetLayouts = pipelineSetLayouts.data(),
            };
            if(vkCreatePipelineLayout(device.getDevice(), &createInfo, nullptr, &computePipelineLayout) != VK_SUCCESS) {
                die("Failed to create pipeline layout");
            }
        }

        auto shaderModule = VkShaderModule{VK_NULL_HANDLE};
        {
            const auto createInfo = VkShaderModuleCreateInfo {
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .codeSize = spirv.size(),
                .pCode    = reinterpret_cast<const uint32_t*>(&spirv[0])
            };
            if(vkCreateShaderModule(device.getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
                die("Failed to create shader module");
            }
        }

        auto pipeline = VkPipeline{VK_NULL_HANDLE};
        {
            const auto shaderStage = VkPipelineShaderStageCreateInfo {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                .module = shaderModule,
                .pName = "main",
                .pSpecializationInfo = nullptr,
            };
            VkComputePipelineCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
            createInfo.stage = shaderStage;
            createInfo.layout = computePipelineLayout;

            if(vkCreateComputePipelines(device.getDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline) != VK_SUCCESS) {
                die("Failed to create compute pipeline");
            }
            vkDestroyShaderModule(device.getDevice(), shaderModule, nullptr);
        }


        const VkDescriptorImageInfo outputTexture = { VK_NULL_HANDLE, cubemapInfo.imageView, VK_IMAGE_LAYOUT_GENERAL };
        auto descriptorSet = VkDescriptorSet{VK_NULL_HANDLE};
        if (!computeDescriptorPool->allocateDescriptor(*descriptorSetLayout->getDescriptorSetLayout(), descriptorSet)) {
            die("Cannot allocate descriptor set");
        }

        auto writer  = DescriptorWriter(*descriptorSetLayout, *computeDescriptorPool)
                                       .writeImage(0, &hdrImageInfo)
                                       .writeImage(1, &outputTexture);
        if (!writer.build(descriptorSet))
            die("Cannot allocate compute descriptor set");

        auto commandBuffer = device.beginOneTimeCommandBuffer();
        {
            // const auto preDispatchBarrier = ImageMemoryBarrier(envTextureUnfiltered, 0, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL).mipLevels(0, 1);
            // pipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, { preDispatchBarrier });

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
            vkCmdDispatch(commandBuffer, cubemap->getWidth()/32, cubemap->getWidth()/32, 6);

            // const auto postDispatchBarrier = ImageMemoryBarrier(envTextureUnfiltered, VK_ACCESS_SHADER_WRITE_BIT, 0, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL).mipLevels(0, 1);
            // pipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, { postDispatchBarrier });
        }
        device.endOneTimeCommandBuffer(commandBuffer);

        vkDestroyPipeline(device.getDevice(), pipeline, nullptr);
        vkDestroyPipelineLayout(device.getDevice(), computePipelineLayout, nullptr);
    }

} // namespace z0
