/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/vulkan.h"
// #include <ktx.h>
// #include <ktxvulkan.h>
#include "z0/libraries.h"

module z0.vulkan.Image;

import z0.Constants;
import z0.Log;
import z0.Tools;
import z0.ZRes;

import z0.resources.Image;

import z0.vulkan.Buffer;
import z0.vulkan.Device;

namespace z0 {


    VulkanImage::VulkanImage(const Device &device,
                 const string &             name,
                 const uint32_t             width,
                 const uint32_t             height,
                 const VkDeviceSize         imageSize,
                 const void *               data,
                 const VkFormat             format,
                 const VkImageTiling        tiling,
                 const VkSamplerAddressMode samplerAddressMode,
                 const VkFilter             samplerFilter,
                 const bool                 noMipmaps,
                 const bool                 isArray):
        VulkanImage(device, name, width, height, imageSize, data, format,
            samplerFilter, samplerFilter,
            samplerAddressMode, samplerAddressMode,
            tiling, noMipmaps, isArray) {
    }

    VulkanImage::VulkanImage(const Device &device,
               const string &             name,
               const uint32_t             width,
               const uint32_t             height,
               const VkDeviceSize         imageSize,
               const void *               data,
               const VkFormat             format,
               const VkFilter             magFiter,
               const VkFilter             minFiler,
               const VkSamplerAddressMode samplerAddressModeU,
               const VkSamplerAddressMode samplerAddressModeV,
               const VkImageTiling        tiling,
               const bool                 noMipmaps,
               const bool                 isArray) :
        Image(width, height, name),
        device{device} {
        DEBUG("VulkanImage uploading ", name);
        const auto command = device.beginOneTimeCommandBuffer();
        const auto& textureStagingBuffer = device.createOneTimeBuffer(
                command,
                imageSize,
                1,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT
        );
        textureStagingBuffer.writeToBuffer(data);

        mipLevels = noMipmaps ? 1 : numMipmapLevels(width, height); //static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
        device.createImage(width,
                           height,
                           mipLevels,
                           VK_SAMPLE_COUNT_1_BIT,
                           format,
                           tiling,
                           VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                           VK_IMAGE_USAGE_SAMPLED_BIT,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                           textureImage,
                           textureImageMemory);

        // https://vulkan-tutorial.com/Texture_mapping/Images#page_Copying-buffer-to-image
        const auto region = VkBufferImageCopy {
                .bufferOffset = 0,
                .bufferRowLength = 0,
                .bufferImageHeight = 0,
                .imageSubresource = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .mipLevel = 0,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                },
                .imageOffset = {0, 0, 0},
                .imageExtent = {width, height, 1},
        };
        Device::transitionImageLayout(command.commandBuffer,
                                      textureImage,
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                      0,
                                      VK_ACCESS_TRANSFER_WRITE_BIT,
                                      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                      VK_PIPELINE_STAGE_TRANSFER_BIT,
                                      VK_IMAGE_ASPECT_COLOR_BIT,
                                      mipLevels);
        vkCmdCopyBufferToImage(
                command.commandBuffer,
                textureStagingBuffer.getBuffer(),
                textureImage,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &region
                );
        if (noMipmaps) {
            Device::transitionImageLayout(command.commandBuffer,
                                       textureImage,
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                       VK_ACCESS_TRANSFER_WRITE_BIT,
                                       0,
                                       VK_PIPELINE_STAGE_TRANSFER_BIT,
                                       VK_PIPELINE_STAGE_TRANSFER_BIT ,
                                       VK_IMAGE_ASPECT_COLOR_BIT,
                                       mipLevels);
        }
        device.endOneTimeCommandBuffer(command);
        textureImageView = device.createImageView(textureImage, format, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels,
            isArray ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D);
        if (!noMipmaps) {
            //transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps
            generateMipmaps(format);
        }

        createTextureSampler(magFiter, minFiler, samplerAddressModeU, samplerAddressModeV);
    }

    VkFormat VulkanImage::formatSRGB(const VkFormat format, const string& name) {
        switch (format) {
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
            return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
            return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
        case VK_FORMAT_BC3_UNORM_BLOCK:
            return VK_FORMAT_BC3_SRGB_BLOCK;
        case VK_FORMAT_BC7_UNORM_BLOCK:
            return VK_FORMAT_BC7_SRGB_BLOCK;
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
        case VK_FORMAT_BC3_SRGB_BLOCK:
        case VK_FORMAT_BC7_SRGB_BLOCK:
            return format;
        default:
            die("Unsupported compressed format for ", name);
            return VK_FORMAT_UNDEFINED;
        }
    }

    VulkanImage::VulkanImage(const Device &device,
              const uint32_t             width,
              const uint32_t             height,
              const uint32_t             layers,
              const VkFormat             format,
              const uint32_t             mipLevels,
              const VkImageUsageFlags    additionalUsage,
              const VkSamplerAddressMode samplerAddressMode,
              const VkFilter             samplerFilter,
              const VkBool32             anisotropyEnable) :
        Image(width, height, "Image"),
        device{device}{
        device.createImage(width,
                           height,
                           mipLevels,
                           VK_SAMPLE_COUNT_1_BIT,
                           format,
                           VK_IMAGE_TILING_OPTIMAL,
                           VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | additionalUsage,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                           textureImage,
                           textureImageMemory,
                        (layers == 6) ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0,
                            layers);
        textureImageView = device.createImageView(textureImage, format, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
        createTextureSampler(samplerFilter, samplerAddressMode, anisotropyEnable);
    }

     VulkanImage::VulkanImage(const Device &  device, const VkCommandBuffer commandBuffer,
              const string &                name,
              const ZRes::ImageHeader &   imageHeader,
              const vector<ZRes::MipLevelInfo>& mipLevelHeaders,
              const ZRes::TextureHeader & textureHeader,
              const Buffer                  &buffer,
              const uint64_t                bufferOffset,
              const VkImageTiling           tiling):
        Image(imageHeader.width, imageHeader.height, name),
        device{device},
        mipLevels{imageHeader.mipLevels} {
        const auto format = static_cast<VkFormat>(imageHeader.format);

        // Setup buffer copy regions for each mip level
        auto copyRegions = vector<VkBufferImageCopy>{};
        for (uint32_t mip_level = 0; mip_level < mipLevels; mip_level++) {
            const auto buffer_copy_region = VkBufferImageCopy{
                .bufferOffset       = bufferOffset + mipLevelHeaders.at(mip_level).offset,
                .imageSubresource {
                    .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel       = mip_level,
                    .layerCount     = 1,
                },
                .imageExtent {
                    .width          = width >> mip_level,
                    .height         = height >> mip_level,
                    .depth          = 1,
                },
            };
            copyRegions.push_back(buffer_copy_region);
        }
        device.createImage(width,
                           height,
                           mipLevels,
                           VK_SAMPLE_COUNT_1_BIT,
                           format,
                           tiling,
                           VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                           VK_IMAGE_USAGE_SAMPLED_BIT,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                           textureImage,
                           textureImageMemory);

        Device::transitionImageLayout(commandBuffer,
                                      textureImage,
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                      0,
                                      VK_ACCESS_TRANSFER_WRITE_BIT,
                                      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                      VK_PIPELINE_STAGE_TRANSFER_BIT,
                                      VK_IMAGE_ASPECT_COLOR_BIT,
                                      mipLevels);
        vkCmdCopyBufferToImage(
                commandBuffer,
                buffer.getBuffer(),
                textureImage,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                copyRegions.size(),
                copyRegions.data());
        Device::transitionImageLayout(commandBuffer,
                                   textureImage,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                   VK_ACCESS_TRANSFER_WRITE_BIT,
                                   0,
                                   VK_PIPELINE_STAGE_TRANSFER_BIT,
                                   VK_PIPELINE_STAGE_TRANSFER_BIT ,
                                   VK_IMAGE_ASPECT_COLOR_BIT,
                                   mipLevels);
        textureImageView = device.createImageView(textureImage, format, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
        createTextureSampler(
            static_cast<VkFilter>(textureHeader.magFilter),
            static_cast<VkFilter>(textureHeader.minFilter),
            static_cast<VkSamplerAddressMode>(textureHeader.samplerAddressModeU),
            static_cast<VkSamplerAddressMode>(textureHeader.samplerAddressModeV));
    }

    VulkanImage::~VulkanImage() {
        DEBUG("~VulkanImage ", getName());
        if (getName() == "Quit") {
            DEBUG("HERE", getName());
        }
        vkDestroySampler(device.getDevice(), textureSampler, nullptr);
        vkDestroyImageView(device.getDevice(), textureImageView, nullptr);
        if (textureImage != VK_NULL_HANDLE) { vkDestroyImage(device.getDevice(), textureImage, nullptr); }
        if (textureImageMemory != VK_NULL_HANDLE) { vkFreeMemory(device.getDevice(), textureImageMemory, nullptr); }
    }

    void VulkanImage::createTextureSampler(
        const VkFilter magFilter,
        const VkFilter minFilter,
        const VkSamplerAddressMode samplerAddressModeU,
        const VkSamplerAddressMode samplerAddressModeV,
        const VkBool32 anisotropyEnable) {
        // https://vulkan-tutorial.com/Texture_mapping/Image_view_and_sampler#page_Samplers
        const VkSamplerCreateInfo samplerInfo{
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .magFilter = magFilter,
                .minFilter = minFilter,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                .addressModeU = samplerAddressModeU,
                .addressModeV = samplerAddressModeV,
                .mipLodBias = 0.0f,
                .anisotropyEnable = anisotropyEnable,
                // https://vulkan-tutorial.com/Texture_mapping/Image_view_and_sampler#page_Anisotropy-device-feature
                .maxAnisotropy = device.getDeviceProperties().limits.maxSamplerAnisotropy,
                .compareEnable = VK_FALSE,
                .compareOp = VK_COMPARE_OP_ALWAYS,
                .minLod = 0.0f,
                .maxLod = static_cast<float>(mipLevels),
                .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
                .unnormalizedCoordinates = VK_FALSE,
        };
        if (vkCreateSampler(device.getDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
            die("failed to create texture sampler!");
        }
    }

    void VulkanImage::generateMipmaps(const VkFormat imageFormat) const {
        // https://vulkan-tutorial.com/en/Generating_Mipmaps
        // Check if image format supports linear blitting
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(device.getPhysicalDevice(), imageFormat, &formatProperties);
        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
            die("texture image format does not support linear blitting!"); // TODO
        }
        const auto commandBuffer = device.beginOneTimeCommandBuffer();

        VkImageMemoryBarrier barrier{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = textureImage,
                .subresourceRange = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                }
        };

        auto mipWidth  = static_cast<int32_t>(width);
        auto mipHeight = static_cast<int32_t>(height);
        for (uint32_t i = 1; i < mipLevels; i++) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout                     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask                 = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask                 = VK_ACCESS_TRANSFER_READ_BIT;
            vkCmdPipelineBarrier(commandBuffer.commandBuffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 0,
                                 nullptr,
                                 1,
                                 &barrier);

            VkImageBlit blit{};
            blit.srcOffsets[0] = {0, 0, 0};
            blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = {0, 0, 0};
            blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;
            vkCmdBlitImage(commandBuffer.commandBuffer,
                           textureImage,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           textureImage,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &blit,
                           VK_FILTER_LINEAR);

            barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            vkCmdPipelineBarrier(commandBuffer.commandBuffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT ,
                                 0,
                                 0,
                                 nullptr,
                                 0,
                                 nullptr,
                                 1,
                                 &barrier);

            if (mipWidth > 1)
                mipWidth /= 2;
            if (mipHeight > 1)
                mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout                     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask                 = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask                 = VK_ACCESS_TRANSFER_READ_BIT;
        vkCmdPipelineBarrier(commandBuffer.commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT ,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &barrier);

        device.endOneTimeCommandBuffer(commandBuffer);
    }

    // ktxVulkanDeviceInfo KTXVulkanImage::vdi;
    //
    // void KTXVulkanImage::initialize(const VkPhysicalDevice physicalDevice, const VkDevice device, const VkQueue queue, const VkCommandPool cmdPool) {
    //     ktxVulkanDeviceInfo_Construct(&vdi, physicalDevice, device, queue, cmdPool, nullptr);
    // }
    //
    // void KTXVulkanImage::cleanup() {
    //     ktxVulkanDeviceInfo_Destruct(&vdi);
    // }
    //
    //  KTXVulkanImage::KTXVulkanImage(const Device &  device,
    //           const string &             name,
    //           ktxTexture2*               kTexture,
    //           const VkFilter             magFilter,
    //           const VkFilter             minFilter,
    //           const VkSamplerAddressMode samplerAddressModeU,
    //           const VkSamplerAddressMode samplerAddressModeV,
    //           const bool                 forceSRGB,
    //           const VkImageTiling        tiling):
    //     VulkanImage(device, kTexture->baseWidth, kTexture->baseHeight, kTexture->numLevels, name) {
    //     if (forceSRGB) {
    //         kTexture->vkFormat = formatSRGB(static_cast<VkFormat>(kTexture->vkFormat), name);
    //     }
    //     if (KTX_SUCCESS  != ktxTexture2_VkUploadEx(kTexture,
    //                               &vdi, &texture,
    //                               tiling,
    //                               VK_IMAGE_USAGE_SAMPLED_BIT,
    //                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)) {
    //         die("Failed to create Vulkan image from KTX texture ", name);
    //     }
    //     textureImage = texture.image;
    //     textureImageView = device.createImageView(textureImage, texture.imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
    //     createTextureSampler(magFilter, minFilter, samplerAddressModeU, samplerAddressModeV);
    // }
    //
    // KTXVulkanImage::~KTXVulkanImage() {
    //     ktxVulkanTexture_Destruct(&texture, device.getDevice(), nullptr);
    //     textureImage = VK_NULL_HANDLE;
    // }

}
