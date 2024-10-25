module;
#include <volk.h>
#include "z0/libraries.h"

module z0;

import :Constants;
import :Tools;
import :Device;
import :Image;
import :Cubemap;
import :IBLPipeline;
import :Descriptors;

namespace z0 {

    IBLPipeline::IBLPipeline(Device &device) : ComputePipeline{device} {
        descriptorPool =  DescriptorPool::Builder(device)
                           .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1) // HDRi input image
                           .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1) // Cubemap image
                           .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, Cubemap::ENVIRONMENT_MAP_MIPMAP_LEVELS - 1) // Env maps
                           .build();
        descriptorSetLayout = DescriptorSetLayout::Builder(device)
                    // HDRi input image
                   .addBinding(0,
                               VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                               VK_SHADER_STAGE_COMPUTE_BIT,
                               1)
                    // Cubemap image
                   .addBinding(1,
                               VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                               VK_SHADER_STAGE_COMPUTE_BIT,
                               1)
                    // Env maps
                    .addBinding(2,
                                VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                VK_SHADER_STAGE_COMPUTE_BIT,
                                Cubemap::ENVIRONMENT_MAP_MIPMAP_LEVELS-1)
                   .build();
        pipelineLayout = createPipelineLayout(*descriptorSetLayout->getDescriptorSetLayout());
    }

    void IBLPipeline::convert(const shared_ptr<Image>&   hdrFile,
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
        pipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            vector{
                imageMemoryBarrier(
                    cubemap->getImage(),
                    0, VK_ACCESS_SHADER_WRITE_BIT,
                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
                    0, 1)
        });
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
        vkCmdDispatch(commandBuffer, cubemap->getWidth()/32, cubemap->getWidth()/32, 6);
        pipelineBarrier(
                   commandBuffer,
                   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                   VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                   vector{
                       imageMemoryBarrier(
                           cubemap->getImage(),
                           VK_ACCESS_SHADER_WRITE_BIT, 0,
                           VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           0, 1)
               });
        device.endOneTimeCommandBuffer(commandBuffer);
        vkDestroyPipeline(device.getDevice(), pipeline, nullptr);
    }
} // namespace z0
