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
        // Linear, non-anisotropic sampler, wrap address mode
        constexpr auto createInfo = VkSamplerCreateInfo {
            .sType =  VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        };
        if(vkCreateSampler(device.getDevice(), &createInfo, nullptr, &computeSampler) != VK_SUCCESS)
            die("Failed to create pre-processing sampler");
        descriptorPool =  DescriptorPool::Builder(device)
                           .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1) // HDRi input image
                           .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1) // Cubemap image
                           .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, EnvironmentCubemap::ENVIRONMENT_MAP_MIPMAP_LEVELS - 1) // Env maps
                           .build();
        descriptorSetLayout = DescriptorSetLayout::Builder(device)
                                      // HDRi input image
                                      .addBinding(BINDING_INPUT_TEXTURE,
                                                  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                  VK_SHADER_STAGE_COMPUTE_BIT,
                                                  1,
                                                  &computeSampler)
                                      // Cubemap image
                                      .addBinding(BINDING_OUTPUT_TEXTURE,
                                                  VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                  VK_SHADER_STAGE_COMPUTE_BIT,
                                                  1)
                                      // Env maps
                                      .addBinding(BINDING_OUTPUT_MIPMAPS,
                                                  VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                  VK_SHADER_STAGE_COMPUTE_BIT,
                                                  EnvironmentCubemap::ENVIRONMENT_MAP_MIPMAP_LEVELS - 1)
                                      .build();
        constexpr auto pipelinePushConstantRanges = VkPushConstantRange{
            VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(SpecularFilterPushConstants),
        };
        pipelineLayout = createPipelineLayout(*descriptorSetLayout->getDescriptorSetLayout(), &pipelinePushConstantRanges);
        if (!descriptorPool->allocateDescriptor(*descriptorSetLayout->getDescriptorSetLayout(), descriptorSet))
            die("Cannot allocate descriptor set");
    }

    IBLPipeline::~IBLPipeline() {
        vkDestroySampler(device.getDevice(), computeSampler, nullptr);
    }

    void IBLPipeline::convert(const shared_ptr<Image>&   hdrFile,
                              const shared_ptr<Cubemap>& cubemap) const {
        const auto shaderModule = createShaderModule(readFile("equirect2cube.comp"));
        const auto pipeline = createPipeline(shaderModule);

        const auto inputInfo = VkDescriptorImageInfo{ VK_NULL_HANDLE, hdrFile->_getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
        const auto outputInfo = VkDescriptorImageInfo{ VK_NULL_HANDLE, cubemap->_getImageView(), VK_IMAGE_LAYOUT_GENERAL };
        DescriptorWriter(*descriptorSetLayout, *descriptorPool)
            .writeImage(BINDING_INPUT_TEXTURE, &inputInfo)
            .writeImage(BINDING_OUTPUT_TEXTURE, &outputInfo)
            .update(descriptorSet);

        const auto commandBuffer = device.beginOneTimeCommandBuffer();
        pipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            {
                imageMemoryBarrier(
                    cubemap->_getImage(),
                    0, VK_ACCESS_SHADER_WRITE_BIT,
                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL)
        });
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
        vkCmdDispatch(commandBuffer, cubemap->getWidth()/32, cubemap->getWidth()/32, 6);
        pipelineBarrier(commandBuffer,
                   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                   {
                       imageMemoryBarrier(
                           cubemap->_getImage(),
                           VK_ACCESS_SHADER_WRITE_BIT, 0,
                           VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        });
        device.endOneTimeCommandBuffer(commandBuffer);
        vkDestroyPipeline(device.getDevice(), pipeline, nullptr);
    }

    void IBLPipeline::preComputeSpecular(const shared_ptr<Cubemap>& unfilteredCubemap, const shared_ptr<Cubemap>& cubemap) const {
        const auto shaderModule = createShaderModule(readFile("specular_map.comp"));
        const auto pipeline = createPipeline(shaderModule, &specializationInfo);
        const auto commandBuffer = device.beginOneTimeCommandBuffer();
        // Copy base mipmap level into destination environment map.
        {
            pipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,
                {
                    imageMemoryBarrier(
                        unfilteredCubemap->_getImage(),
                        0, VK_ACCESS_TRANSFER_READ_BIT,
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL),
                    imageMemoryBarrier(
                        cubemap->_getImage(),
                        0, VK_ACCESS_TRANSFER_WRITE_BIT,
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
            });

            auto copyRegion = VkImageCopy {
                .srcSubresource = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .layerCount = 6,
                },
                .extent = { cubemap->getWidth(), cubemap->getHeight(), 1 },
            };
            copyRegion.dstSubresource = copyRegion.srcSubresource;
            vkCmdCopyImage(commandBuffer,
                unfilteredCubemap->_getImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                cubemap->_getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &copyRegion);

            pipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                {
                    imageMemoryBarrier(
                       unfilteredCubemap->_getImage(),
                       VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
                    imageMemoryBarrier(
                       cubemap->_getImage(),
                       VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_WRITE_BIT,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL)
            });
        }
        // Pre-filter rest of the mip-chain.
       auto envTextureMipTailViews = vector<VkImageView>{};
       {
            const auto inputInfo = VkDescriptorImageInfo{ VK_NULL_HANDLE, unfilteredCubemap->_getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
            auto outputInfo = vector<VkDescriptorImageInfo>{};
            for(auto level = 1; level < EnvironmentCubemap::ENVIRONMENT_MAP_MIPMAP_LEVELS; level++) {
                envTextureMipTailViews.push_back(device.createImageView(cubemap->_getImage(),
                                                  cubemap->_getFormat(),
                                                  VK_IMAGE_ASPECT_COLOR_BIT,
                                                  1,
                                                  VK_IMAGE_VIEW_TYPE_CUBE,
                                                  0,
                                                  VK_REMAINING_ARRAY_LAYERS,
                                                  level));
                outputInfo.push_back(VkDescriptorImageInfo{ VK_NULL_HANDLE, envTextureMipTailViews[level-1], VK_IMAGE_LAYOUT_GENERAL });
            }

            DescriptorWriter(*descriptorSetLayout, *descriptorPool)
                .writeImage(BINDING_INPUT_TEXTURE, &inputInfo)
                .writeImage(BINDING_OUTPUT_MIPMAPS, outputInfo.data())
                .update(descriptorSet);

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout,
                                    0, 1, &descriptorSet,
                                    0, nullptr);

            constexpr auto deltaRoughness = 1.0f / std::max(static_cast<float>(EnvironmentCubemap::ENVIRONMENT_MAP_MIPMAP_LEVELS - 1), 1.0f);
            for(uint32_t level = 1, size = EnvironmentCubemap::ENVIRONMENT_MAP_SIZE/2; level<EnvironmentCubemap::ENVIRONMENT_MAP_MIPMAP_LEVELS; ++level, size /= 2) {
                const auto numGroups = std::max<uint32_t>(1, size/32);
                const auto pushConstants = SpecularFilterPushConstants{ level-1, level * deltaRoughness };
                vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(SpecularFilterPushConstants), &pushConstants);
                vkCmdDispatch(commandBuffer, numGroups, numGroups, 6);
            }

            pipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                vector{
                    imageMemoryBarrier(cubemap->_getImage(),
                        VK_ACCESS_SHADER_WRITE_BIT, 0,
                        VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            });
        }
        device.endOneTimeCommandBuffer(commandBuffer);
        for (const VkImageView mipTailView : envTextureMipTailViews) {
            vkDestroyImageView(device.getDevice(), mipTailView, nullptr);
        }
        vkDestroyPipeline(device.getDevice(), pipeline, nullptr);
    }

    void IBLPipeline::preComputeIrradiance(const shared_ptr<Cubemap>& cubemap, const shared_ptr<Cubemap>& irradianceCubemap) const {
        const auto shaderModule = createShaderModule(readFile("irradiance_map.comp"));
        const auto pipeline = createPipeline(shaderModule);

        const auto inputInfo = VkDescriptorImageInfo{ VK_NULL_HANDLE, cubemap->_getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
        const auto outputInfo = VkDescriptorImageInfo{ VK_NULL_HANDLE, irradianceCubemap->_getImageView(), VK_IMAGE_LAYOUT_GENERAL };
        DescriptorWriter(*descriptorSetLayout, *descriptorPool)
            .writeImage(BINDING_INPUT_TEXTURE, &inputInfo)
            .writeImage(BINDING_OUTPUT_TEXTURE, &outputInfo)
            .update(descriptorSet);

        // Compute diffuse irradiance cubemap
        const auto commandBuffer = device.beginOneTimeCommandBuffer();
        pipelineBarrier(commandBuffer,
           VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
           {
               imageMemoryBarrier(
                   irradianceCubemap->_getImage(),
                   0, VK_ACCESS_SHADER_WRITE_BIT,
                   VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL)
        });
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout,
            0, 1, &descriptorSet,
            0, nullptr);
        vkCmdDispatch(commandBuffer,
            EnvironmentCubemap::IRRADIANCE_MAP_SIZE/32,
            EnvironmentCubemap::IRRADIANCE_MAP_SIZE/32,
            6);
        pipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
           {
               imageMemoryBarrier(
                   irradianceCubemap->_getImage(),
                   VK_ACCESS_SHADER_WRITE_BIT, 0,
                   VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        });
        device.endOneTimeCommandBuffer(commandBuffer);
        vkDestroyPipeline(device.getDevice(), pipeline, nullptr);
    }

    void IBLPipeline::preComputeBRDF(const shared_ptr<Image>& brdfLut) const {
        const auto shaderModule = createShaderModule(readFile("brdf.comp"));
        const auto pipeline = createPipeline(shaderModule);
        const auto outputInfo = VkDescriptorImageInfo{ VK_NULL_HANDLE, brdfLut->_getImageView(), VK_IMAGE_LAYOUT_GENERAL };
        DescriptorWriter(*descriptorSetLayout, *descriptorPool)
            // input texture already set in preComputeIrradiance()
            .writeImage(BINDING_OUTPUT_TEXTURE, &outputInfo)
            .update(descriptorSet);
		// Compute Cook-Torrance BRDF 2D LUT for split-sum approximation.
        const auto commandBuffer = device.beginOneTimeCommandBuffer();
        pipelineBarrier(commandBuffer,
          VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
          {
              imageMemoryBarrier(
                  brdfLut->_getImage(),
                  0, VK_ACCESS_SHADER_WRITE_BIT,
                  VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL)
        });
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
        vkCmdBindDescriptorSets(commandBuffer,
            VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout,
            0, 1, &descriptorSet,
            0, nullptr);
        vkCmdDispatch(commandBuffer,
            EnvironmentCubemap::BRDFLUT_SIZE/32, EnvironmentCubemap::BRDFLUT_SIZE/32, 6);
        pipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            {
               imageMemoryBarrier(
                   brdfLut->_getImage(),
                   VK_ACCESS_SHADER_WRITE_BIT, 0,
                   VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        });
        device.endOneTimeCommandBuffer(commandBuffer);
        vkDestroyPipeline(device.getDevice(), pipeline, nullptr);
    }
} // namespace z0
