module;
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/util.hpp>
#include "stb_image.h"
#include "stb_image_write.h"
#include "z0/libraries.h"
#include <volk.h>

module z0;

import :Tools;
import :Device;
import :Buffer;
import :Application;
import :Image;

namespace z0 {

    Image::Image(const Device &             device,
                 const string &             name,
                 const uint32_t             width,
                 const uint32_t             height,
                 const VkDeviceSize         imageSize,
                 const void *               data,
                 const VkFormat             format,
                 const VkImageTiling        tiling,
                 const VkSamplerAddressMode samplerAddressMode,
                 const VkFilter             samplerFilter,
                 const bool                 noMipmaps):
        Resource(name),
        device{device},
        width{width},
        height{height} {
        const Buffer textureStagingBuffer{
                device,
                imageSize,
                1,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        };
        textureStagingBuffer.writeToBuffer(data);

        mipLevels = noMipmaps ? 0 : static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
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
        const VkBufferImageCopy region{
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
        const VkCommandBuffer commandBuffer = device.beginOneTimeCommandBuffer();
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
                textureStagingBuffer.getBuffer(),
                textureImage,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &region
                );
        device.endOneTimeCommandBuffer(commandBuffer);
        //transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps
        textureImageView = device.createImageView(textureImage, format, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);

        if (!noMipmaps) { generateMipmaps(format); }
        createTextureSampler(samplerFilter, samplerAddressMode);
    }

    Image::Image(const Device &       device,
              const uint32_t             width,
              const uint32_t             height,
              const uint32_t             layers,
              const VkFormat             format,
              const uint32_t             mipLevels,
              const VkImageUsageFlags    additionalUsage,
              const VkSamplerAddressMode samplerAddressMode,
              const VkFilter             samplerFilter,
              const VkBool32             anisotropyEnable) :
        Resource("Image"),
        device{device},
        width{width},
        height{height} {
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

    Image::~Image() {
        vkDestroySampler(device.getDevice(), textureSampler, nullptr);
        vkDestroyImageView(device.getDevice(), textureImageView, nullptr);
        vkDestroyImage(device.getDevice(), textureImage, nullptr);
        vkFreeMemory(device.getDevice(), textureImageMemory, nullptr);
    }

    void vr_stb_write_func(void *context, void *data, const int size) {
        auto *buffer = static_cast<vector<unsigned char> *>(context);
        auto *ptr    = static_cast<unsigned char *>(data);
        buffer->insert(buffer->end(), ptr, ptr + size);
    }

    unique_ptr<Image> Image::createBlankImage() {
        vector<unsigned char> blankJPEG;
        const auto data = new unsigned char[1 * 1 * 3];
        data[0]   = 0;
        data[1]   = 0;
        data[2]   = 0;
        stbi_write_jpg_to_func(vr_stb_write_func, &blankJPEG, 1, 1, 3, data, 100);
        delete[] data;
        return make_unique<Image>(Application::get()._getDevice(), "Blank", 1, 1, blankJPEG.size(), blankJPEG.data());
    }

    shared_ptr<Image> Image::loadFromFile(const string &filename) {
        const auto &filepath = (Application::get().getConfig().appDir / filename).string();
        // Create texture image
        // https://vulkan-tutorial.com/Texture_mapping/Images#page_Loading-an-image
        int   texWidth, texHeight, texChannels;
        auto *pixels = stbi_load(filepath.c_str(),
                                 &texWidth,
                                 &texHeight,
                                 &texChannels,
                                 STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * STBI_rgb_alpha;
        if (!pixels)
            die("failed to load texture image!");
        auto image = make_shared<Image>(Application::get()._getDevice(),
                                        filepath,
                                        static_cast<uint32_t>(texWidth),
                                        static_cast<uint32_t>(texHeight),
                                        imageSize,
                                        static_cast<void *>(pixels));
        stbi_image_free(pixels);
        return image;
    }

    void Image::createTextureSampler(const VkFilter samplerFilter, const VkSamplerAddressMode samplerAddressMode, const VkBool32 anisotropyEnable) {
        // https://vulkan-tutorial.com/Texture_mapping/Image_view_and_sampler#page_Samplers
        const VkSamplerCreateInfo samplerInfo{
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .magFilter = samplerFilter,
                .minFilter = samplerFilter,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                .addressModeU = samplerAddressMode,
                .addressModeV = samplerAddressMode,
                .addressModeW = samplerAddressMode,
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

    void Image::generateMipmaps(const VkFormat imageFormat) const {
        // https://vulkan-tutorial.com/en/Generating_Mipmaps
        // Check if image format supports linear blitting
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(device.getPhysicalDevice(), imageFormat, &formatProperties);
        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
            die("texture image format does not support linear blitting!"); // TODO
        }
        const VkCommandBuffer commandBuffer = device.beginOneTimeCommandBuffer();

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
            vkCmdPipelineBarrier(commandBuffer,
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
            vkCmdBlitImage(commandBuffer,
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
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            vkCmdPipelineBarrier(commandBuffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
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
        barrier.srcAccessMask                 = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask                 = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &barrier);

        device.endOneTimeCommandBuffer(commandBuffer);
    }

}
