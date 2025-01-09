/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>
#include "z0/libraries.h"

module z0.vulkan.IBLPipeline;

import z0.Constants;
import z0.Tools;
import z0.VirtualFS;

import z0.resources.Cubemap;
import z0.resources.Image;

import z0.vulkan.Descriptors;
import z0.vulkan.Device;
import z0.vulkan.Cubemap;
import z0.vulkan.Image;

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
    }

    IBLPipeline::~IBLPipeline() {
        vkDestroySampler(device.getDevice(), computeSampler, nullptr);
    }

    void IBLPipeline::convert(const shared_ptr<VulkanImage>&    hdrFile,
                              const shared_ptr<VulkanCubemap>&  unfilteredCubemap,
                              const shared_ptr<VulkanCubemap>&  filteredCubemap,
                              const shared_ptr<VulkanCubemap>&  irradianceCubemap,
                              const shared_ptr<VulkanImage>&    brdfLut) const {

        {
            const auto shaderModule1 = createShaderModule( VirtualFS::loadShader("equirect2cube.comp"));
            const auto pipeline1 = createPipeline(shaderModule1);
            auto descriptorSet1 = VkDescriptorSet{VK_NULL_HANDLE};
            if (!descriptorPool->allocateDescriptor(*descriptorSetLayout->getDescriptorSetLayout(), descriptorSet1)) {
                die("Cannot allocate descriptor set");
            }

            const auto inputInfo1 = VkDescriptorImageInfo{ VK_NULL_HANDLE, hdrFile->getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
            const auto outputInfo1 = VkDescriptorImageInfo{ VK_NULL_HANDLE, unfilteredCubemap->getImageView(), VK_IMAGE_LAYOUT_GENERAL };
            DescriptorWriter(*descriptorSetLayout, *descriptorPool)
                .writeImage(BINDING_INPUT_TEXTURE, &inputInfo1)
                .writeImage(BINDING_OUTPUT_TEXTURE, &outputInfo1)
                .update(descriptorSet1);

            const auto command = device.beginOneTimeCommandBuffer();
            const auto commandBuffer = command.commandBuffer;
            pipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                {
                    imageMemoryBarrier(
                        unfilteredCubemap->getImage(),
                        0, VK_ACCESS_SHADER_WRITE_BIT,
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL)
            });
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline1);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet1, 0, nullptr);
            vkCmdDispatch(commandBuffer, unfilteredCubemap->getWidth()/32, unfilteredCubemap->getWidth()/32, 6);
            pipelineBarrier(commandBuffer,
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                       {
                           imageMemoryBarrier(
                               unfilteredCubemap->getImage(),
                               VK_ACCESS_SHADER_WRITE_BIT, 0,
                               VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
            });
            device.endOneTimeCommandBuffer(command, true);
            vkDestroyPipeline(device.getDevice(), pipeline1, nullptr);
        }

        {
            const auto command = device.beginOneTimeCommandBuffer();
            const auto commandBuffer = command.commandBuffer;
            const auto shaderModule2 = createShaderModule(VirtualFS::loadShader("specular_map.comp"));
            const auto pipeline2 = createPipeline(shaderModule2, &specializationInfo);
            // Copy base mipmap level into destination environment map.
            {
                pipelineBarrier(commandBuffer,
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,
                    {
                        imageMemoryBarrier(
                            unfilteredCubemap->getImage(),
                            0, VK_ACCESS_TRANSFER_READ_BIT,
                            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL),
                        imageMemoryBarrier(
                            filteredCubemap->getImage(),
                            0, VK_ACCESS_TRANSFER_WRITE_BIT,
                            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
                });

                auto copyRegion = VkImageCopy {
                    .srcSubresource = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .layerCount = 6,
                    },
                    .extent = { filteredCubemap->getWidth(), filteredCubemap->getHeight(), 1 },
                };
                copyRegion.dstSubresource = copyRegion.srcSubresource;
                vkCmdCopyImage(commandBuffer,
                    unfilteredCubemap->getImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    filteredCubemap->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1, &copyRegion);

                pipelineBarrier(commandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                    {
                        imageMemoryBarrier(
                           unfilteredCubemap->getImage(),
                           VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
                        imageMemoryBarrier(
                           filteredCubemap->getImage(),
                           VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_WRITE_BIT,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL)
                });
            }
            // Pre-filter rest of the mip-chain.
            auto envTextureMipTailViews = vector<VkImageView>{};
            const auto inputInfo = VkDescriptorImageInfo{ VK_NULL_HANDLE, unfilteredCubemap->getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
            auto outputInfo = vector<VkDescriptorImageInfo>{};
            for(auto level = 1; level < EnvironmentCubemap::ENVIRONMENT_MAP_MIPMAP_LEVELS; level++) {
                envTextureMipTailViews.push_back(device.createImageView(filteredCubemap->getImage(),
                                                  filteredCubemap->getFormat(),
                                                  VK_IMAGE_ASPECT_COLOR_BIT,
                                                  1,
                                                  VK_IMAGE_VIEW_TYPE_CUBE,
                                                  0,
                                                  VK_REMAINING_ARRAY_LAYERS,
                                                  level));
                outputInfo.push_back(VkDescriptorImageInfo{ VK_NULL_HANDLE, envTextureMipTailViews[level-1], VK_IMAGE_LAYOUT_GENERAL });
            }

            auto descriptorSet2 = VkDescriptorSet{VK_NULL_HANDLE};
            if (!descriptorPool->allocateDescriptor(*descriptorSetLayout->getDescriptorSetLayout(), descriptorSet2)) {
                die("Cannot allocate descriptor set");
            }
            DescriptorWriter(*descriptorSetLayout, *descriptorPool)
                .writeImage(BINDING_INPUT_TEXTURE, &inputInfo)
                .writeImage(BINDING_OUTPUT_MIPMAPS, outputInfo.data())
                .update(descriptorSet2);

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline2);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout,
                                    0, 1, &descriptorSet2,
                                    0, nullptr);

            const auto deltaRoughness = 1.0f / std::max(static_cast<float>(EnvironmentCubemap::ENVIRONMENT_MAP_MIPMAP_LEVELS - 1), 1.0f);
            for(uint32_t level = 1, size = EnvironmentCubemap::ENVIRONMENT_MAP_SIZE/2; level<EnvironmentCubemap::ENVIRONMENT_MAP_MIPMAP_LEVELS; ++level, size /= 2) {
                const auto numGroups = std::max<uint32_t>(1, size/32);
                const auto pushConstants = SpecularFilterPushConstants{ level-1, level * deltaRoughness };
                vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(SpecularFilterPushConstants), &pushConstants);
                vkCmdDispatch(commandBuffer, numGroups, numGroups, 6);
            }

            pipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                vector{
                    imageMemoryBarrier(filteredCubemap->getImage(),
                        VK_ACCESS_SHADER_WRITE_BIT, 0,
                        VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            });
            device.endOneTimeCommandBuffer(command, true);
            for (const VkImageView mipTailView : envTextureMipTailViews) {
                vkDestroyImageView(device.getDevice(), mipTailView, nullptr);
            }
            vkDestroyPipeline(device.getDevice(), pipeline2, nullptr);
        }

        {
            const auto shaderModule3 = createShaderModule(VirtualFS::loadShader("irradiance_map.comp"));
            const auto pipeline3 = createPipeline(shaderModule3);
            auto descriptorSet3 = VkDescriptorSet{VK_NULL_HANDLE};
            if (!descriptorPool->allocateDescriptor(*descriptorSetLayout->getDescriptorSetLayout(), descriptorSet3)) {
                die("Cannot allocate descriptor set");
            }

            const auto inputInfo3 = VkDescriptorImageInfo{ VK_NULL_HANDLE, filteredCubemap->getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
            const auto outputInfo3 = VkDescriptorImageInfo{ VK_NULL_HANDLE, irradianceCubemap->getImageView(), VK_IMAGE_LAYOUT_GENERAL };
            DescriptorWriter(*descriptorSetLayout, *descriptorPool)
                .writeImage(BINDING_INPUT_TEXTURE, &inputInfo3)
                .writeImage(BINDING_OUTPUT_TEXTURE, &outputInfo3)
                .update(descriptorSet3);

            const auto command = device.beginOneTimeCommandBuffer();
            const auto commandBuffer = command.commandBuffer;

            // Compute diffuse irradiance cubemap
            pipelineBarrier(commandBuffer,
               VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
               {
                   imageMemoryBarrier(
                       irradianceCubemap->getImage(),
                       0, VK_ACCESS_SHADER_WRITE_BIT,
                       VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL)
            });
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline3);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout,
                0, 1, &descriptorSet3,
                0, nullptr);
            vkCmdDispatch(commandBuffer,
                EnvironmentCubemap::IRRADIANCE_MAP_SIZE/32,
                EnvironmentCubemap::IRRADIANCE_MAP_SIZE/32,
                6);
            pipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
               {
                   imageMemoryBarrier(
                       irradianceCubemap->getImage(),
                       VK_ACCESS_SHADER_WRITE_BIT, 0,
                       VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            });
            device.endOneTimeCommandBuffer(command, true);
            vkDestroyPipeline(device.getDevice(), pipeline3, nullptr);
        }

        {
            const auto shaderModule4 = createShaderModule(VirtualFS::loadShader("brdf.comp"));
            const auto pipeline4 = createPipeline(shaderModule4);
            auto descriptorSet4 = VkDescriptorSet{VK_NULL_HANDLE};
            if (!descriptorPool->allocateDescriptor(*descriptorSetLayout->getDescriptorSetLayout(), descriptorSet4)) {
                die("Cannot allocate descriptor set");
            }

            const auto outputInfo4 = VkDescriptorImageInfo{ VK_NULL_HANDLE, brdfLut->getImageView(), VK_IMAGE_LAYOUT_GENERAL };
            DescriptorWriter(*descriptorSetLayout, *descriptorPool)
                // input texture already set in preComputeIrradiance()
                .writeImage(BINDING_OUTPUT_TEXTURE, &outputInfo4)
                .update(descriptorSet4);

            const auto command = device.beginOneTimeCommandBuffer();
            const auto commandBuffer = command.commandBuffer;

            // Compute Cook-Torrance BRDF 2D LUT for split-sum approximation.
            pipelineBarrier(commandBuffer,
              VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
              {
                  imageMemoryBarrier(
                      brdfLut->getImage(),
                      0, VK_ACCESS_SHADER_WRITE_BIT,
                      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL)
            });
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline4);
            vkCmdBindDescriptorSets(commandBuffer,
                VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout,
                0, 1, &descriptorSet4,
                0, nullptr);
            vkCmdDispatch(commandBuffer,
                EnvironmentCubemap::BRDFLUT_SIZE/32, EnvironmentCubemap::BRDFLUT_SIZE/32, 6);
            pipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                {
                   imageMemoryBarrier(
                       brdfLut->getImage(),
                       VK_ACCESS_SHADER_WRITE_BIT, 0,
                       VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            });
            device.endOneTimeCommandBuffer(command, true);
            vkDestroyPipeline(device.getDevice(), pipeline4, nullptr);
        }

    }
} // namespace z0
