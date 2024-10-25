module;
#include <volk.h>
#include "z0/libraries.h"

module z0;

import :Constants;
import :Tools;
import :Device;
import :Image;
import :Cubemap;
import :Equirect2CubemapPipeline;
import :Descriptors;

namespace z0 {

    Equirect2CubemapPipeline::Equirect2CubemapPipeline(Device &device) : Pipeline{device} {
        descriptorPool =  DescriptorPool::Builder(device)
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
        const auto pipeline = createPipeline(shaderModule);

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

            const auto postComputeBarrier = array{VkImageMemoryBarrier {
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
                 static_cast<uint32_t>(postComputeBarrier.size()),
                 postComputeBarrier.data());
        }
        device.endOneTimeCommandBuffer(commandBuffer);
        vkDestroyPipeline(device.getDevice(), pipeline, nullptr);
    }
} // namespace z0
