module;
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/util.hpp>
#include "stb_image.h"
#include "z0/libraries.h"
#include <volk.h>

export module Z0:Image;

import :Tools;
import :Resource;
import :Device;
import :Buffer;
import :Application;

export namespace z0 {

    /**
     * A bitmap resource, stored in GPU memory.
     * Use loadFromFile(const string& filepath) to load a image from a file.
     * Helper class for the Vulkan resources VkImage, VkImageView and VkSampler
     */
    class Image: public Resource {
    public:
        /** 
         * Creates a Image resource and store the image data in GPU memory.
         * Use loadFromFile(const string& filepath) to load a image from a file.
         * @param device : the GPU where the image will be stored
         * @param name : resource name. Informational only.
         * @param width : width in pixels
         * @param height : height in pixels
         * @param imageSize : data size in bytes
         * @param data : the source bitmap stored in CPU memory
         * @param format : data format of the bitmap (https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkFormat.html)
         * @param tiling : image tiling arrangment (https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageTiling.html)
         * @param samplerAddressMode : texture sampler mode (https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSamplerAddressMode.html)
         * @param noMipmaps : do not generate mipmaps
        */
        explicit Image(const Device& dev,
                       const string& name,
                       const uint32_t w,
                       const uint32_t h,
                       const VkDeviceSize imageSize,
                       const void* data,
                       const VkFormat format = VK_FORMAT_R8G8B8A8_SRGB,
                       const VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
                       const VkSamplerAddressMode samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                       const bool noMipmaps = false):
                Resource(name),
                device{dev},
                width{w},
                height{h} {
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
                               VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
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
                    .imageExtent = { width, height, 1 },
            };
            VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();
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
            device.endSingleTimeCommands(commandBuffer);
            //transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps
            textureImageView = device.createImageView(textureImage, format, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);

            if (!noMipmaps) { generateMipmaps(format); }
            createTextureSampler(samplerAddressMode);
        }

        ~Image() override {
            vkDestroySampler(device.getDevice(), textureSampler, nullptr);
            vkDestroyImageView(device.getDevice(), textureImageView, nullptr);
            vkDestroyImage(device.getDevice(), textureImage, nullptr);
            vkFreeMemory(device.getDevice(), textureImageMemory, nullptr);
        }

        /**
         * Returns the width in pixels
         */
        [[nodiscard]] inline uint32_t getWidth() const { return width; }

        /**
         * Returns the height in pixels
         */
        [[nodiscard]] inline uint32_t getHeight() const { return height; }

        /**
         * Returns the size in pixels
         */
        [[nodiscard]] vec2 getSize() const { return vec2{getWidth(), getHeight()}; }

        /**
         * Load a bitmap from file. For supported file formats see https://github.com/nothings/stb/blob/master/stb_image.h
         * @param filepath : path of the file, relative to the application working directory
         */
        [[nodiscard]] static shared_ptr<Image> loadFromFile(const string& filename) {
            const auto& filepath = (Application::get().getConfig().appDir / filename).string();
            // Create texture image
            // https://vulkan-tutorial.com/Texture_mapping/Images#page_Loading-an-image
            int texWidth, texHeight, texChannels;
            auto* pixels = stbi_load(filepath.c_str(),
                                        &texWidth,
                                        &texHeight,
                                        &texChannels,
                                        STBI_rgb_alpha);
            VkDeviceSize imageSize = texWidth * texHeight * STBI_rgb_alpha;
            if (!pixels) die("failed to load texture image!");
            auto image= make_shared<Image>(Application::get()._getDevice(),
                                           filepath,
                                           static_cast<uint32_t>(texWidth),
                                           static_cast<uint32_t>(texHeight),
                                           imageSize,
                                           static_cast<void*>(pixels));
            stbi_image_free(pixels);
            return image;
        }

    private:
        const Device&   device;
        uint32_t        width;
        uint32_t        height;
        uint32_t        mipLevels;
        VkImage         textureImage;
        VkDeviceMemory  textureImageMemory;
        VkImageView     textureImageView;
        VkSampler       textureSampler;

        void createTextureSampler(const VkSamplerAddressMode samplerAddressMode) {
            // https://vulkan-tutorial.com/Texture_mapping/Image_view_and_sampler#page_Samplers
            const VkSamplerCreateInfo samplerInfo{
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .magFilter = VK_FILTER_LINEAR,
                .minFilter = VK_FILTER_LINEAR,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                .addressModeU = samplerAddressMode,
                .addressModeV = samplerAddressMode,
                .addressModeW = samplerAddressMode,
                .mipLodBias = 0.0f, // Optional
                .anisotropyEnable = VK_TRUE, // https://vulkan-tutorial.com/Texture_mapping/Image_view_and_sampler#page_Anisotropy-device-feature
                .maxAnisotropy = device.getDeviceProperties().limits.maxSamplerAnisotropy,
                .compareEnable = VK_FALSE,
                .compareOp = VK_COMPARE_OP_ALWAYS,
                .minLod =  0.0f,
                .maxLod = static_cast<float>(mipLevels),
                .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK ,
                .unnormalizedCoordinates = VK_FALSE,
            };
            if (vkCreateSampler(device.getDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
                die("failed to create texture sampler!");
            }
        }

        void generateMipmaps(const VkFormat imageFormat) const {
            // https://vulkan-tutorial.com/en/Generating_Mipmaps
            // Check if image format supports linear blitting
            VkFormatProperties formatProperties;
            vkGetPhysicalDeviceFormatProperties(device.getPhysicalDevice(), imageFormat, &formatProperties);
            if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
                die("texture image format does not support linear blitting!"); // TODO
            }
            const VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();

            VkImageMemoryBarrier barrier {
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

            auto mipWidth = static_cast<int32_t>(width);
            auto mipHeight = static_cast<int32_t>(height);
            for (uint32_t i = 1; i < mipLevels; i++) {
                barrier.subresourceRange.baseMipLevel = i - 1;
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                vkCmdPipelineBarrier(commandBuffer,
                                     VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                                     0, nullptr,
                                     0, nullptr,
                                     1, &barrier);

                VkImageBlit blit{};
                blit.srcOffsets[0] = { 0, 0, 0 };
                blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
                blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.srcSubresource.mipLevel = i - 1;
                blit.srcSubresource.baseArrayLayer = 0;
                blit.srcSubresource.layerCount = 1;
                blit.dstOffsets[0] = { 0, 0, 0 };
                blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
                blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.dstSubresource.mipLevel = i;
                blit.dstSubresource.baseArrayLayer = 0;
                blit.dstSubresource.layerCount = 1;
                vkCmdBlitImage(commandBuffer,
                               textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               1, &blit,
                               VK_FILTER_LINEAR);

                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                vkCmdPipelineBarrier(commandBuffer,
                                     VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                                     0, nullptr,
                                     0, nullptr,
                                     1, &barrier);

                if (mipWidth > 1) mipWidth /= 2;
                if (mipHeight > 1) mipHeight /= 2;
            }

            barrier.subresourceRange.baseMipLevel = mipLevels - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            vkCmdPipelineBarrier(commandBuffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                                 0, nullptr,
                                 0, nullptr,
                                 1, &barrier);

            device.endSingleTimeCommands(commandBuffer);
        }

    public:
        VkDescriptorImageInfo _getImageInfo() const {
            // https://vulkan-tutorial.com/Texture_mapping/Combined_image_sampler#page_Updating-the-descriptors
            return VkDescriptorImageInfo {
                .sampler = textureSampler,
                .imageView = textureImageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
        }
    };

}