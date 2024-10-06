module;
#include <cassert>
#include "stb_image.h"
#include "z0/libraries.h"
#include <volk.h>

module z0;

import :Buffer;
import :Application;
import :Cubemap;
import :Device;

namespace z0 {

    Cubemap::Cubemap(const Device &                 device,
                     const uint32_t                 width,
                     const uint32_t                 height,
                     const VkDeviceSize             imageSize,
                     const vector<unsigned char *> &data,
                     const string &                 name):
        Resource(name), device{device}, width{width}, height{height} {
        assert(data.size() == 6 && "Must have 6 images for a cubemap");
        // Create staging buffer for CPU to GPU images copy
        Buffer textureStagingBuffer{
                device,
                imageSize,
                6,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                device.getDeviceProperties().limits.minUniformBufferOffsetAlignment
        };
        if (textureStagingBuffer.map() != VK_SUCCESS) {
            die("Failed to map Cubmap texture to GPU memory");
        }
        // Copy the 6 images data into staging buffer
        for (int i = 0; i < 6; i++) {
            textureStagingBuffer.writeToBuffer(data[i], imageSize, textureStagingBuffer.getAlignmentSize() * i);
        }

        // Create image in GPU memory, one image in memory for the 6 images of the cubemap
        constexpr VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
        device.createImage(width,
                           height,
                           1,
                           VK_SAMPLE_COUNT_1_BIT,
                           format,
                           VK_IMAGE_TILING_OPTIMAL,
                           VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                           VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                           textureImage,
                           textureImageMemory,
                           VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
                           6);

        // Define the cube map 6 images regions inside the single GPU image
        VkBufferImageCopy layerRegions[6] = {};
        for (uint32_t i = 0; i < 6; i++) {
            layerRegions[i].bufferOffset      = textureStagingBuffer.getAlignmentSize() * i;
            layerRegions[i].bufferRowLength   = 0; // Tightly packed
            layerRegions[i].bufferImageHeight = 0; // Tightly packed
            layerRegions[i].imageSubresource  = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = i,
                    .layerCount = 1,
            };
            layerRegions[i].imageOffset = {0, 0, 0};
            layerRegions[i].imageExtent = {
                    width,
                    height,
                    1
            };
        }
        // prepare for CPU to GPU transfert
        const VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();
        Device::transitionImageLayout(commandBuffer,
                                      textureImage,
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                      0,
                                      VK_ACCESS_TRANSFER_WRITE_BIT,
                                      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                      VK_PIPELINE_STAGE_TRANSFER_BIT,
                                      VK_IMAGE_ASPECT_COLOR_BIT);
        // Copy the 6 images into the GPU
        vkCmdCopyBufferToImage(
                commandBuffer,
                textureStagingBuffer.getBuffer(),
                textureImage,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                6,
                layerRegions
                );
        // End the transfert
        Device::transitionImageLayout(commandBuffer,
                                      textureImage,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                      VK_ACCESS_TRANSFER_WRITE_BIT,
                                      VK_ACCESS_SHADER_READ_BIT,
                                      VK_PIPELINE_STAGE_TRANSFER_BIT,
                                      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                      VK_IMAGE_ASPECT_COLOR_BIT);
        device.endSingleTimeCommands(commandBuffer);

        textureImageView = device.createImageView(textureImage,
                                                  format,
                                                  VK_IMAGE_ASPECT_COLOR_BIT,
                                                  1,
                                                  VK_IMAGE_VIEW_TYPE_CUBE);
        createTextureSampler();
    }

    Cubemap::~Cubemap() {
        vkDestroySampler(device.getDevice(), textureSampler, nullptr);
        vkDestroyImageView(device.getDevice(), textureImageView, nullptr);
        vkDestroyImage(device.getDevice(), textureImage, nullptr);
        vkFreeMemory(device.getDevice(), textureImageMemory, nullptr);
    }

    shared_ptr<Cubemap> Cubemap::loadFromFile(const string &filename, const string &fileext) {
        const auto &                filepath = (Application::get().getConfig().appDir / filename).string();
        int                         texWidth, texHeight, texChannels;
        vector<unsigned char *>     data;
        const array<std::string, 6> names{"right", "left", "top", "bottom", "front", "back"};
        for (int i = 0; i < 6; i++) {
            string   path   = filepath + "_" + names[i] + fileext;
            stbi_uc *pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            if (!pixels) {
                die("failed to load texture image", path);
            }
            data.push_back(pixels);
        }
        auto cubemap = make_shared<Cubemap>(Application::get()._getDevice(),
                                            texWidth,
                                            texHeight,
                                            texWidth * texHeight * STBI_rgb_alpha,
                                            data);
        for (int i = 0; i < 6; i++) {
            stbi_image_free(data[i]);
        }
        return cubemap;
    }

    shared_ptr<Cubemap> Cubemap::loadFromFile(const string &filename) {
        const auto &filepath = (Application::get().getConfig().appDir / filename).string();
        int         texWidth, texHeight, texChannels;
        stbi_uc *   pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if (!pixels) {
            die("failed to load texture image", filepath);
        }
        vector<unsigned char *> data;
        const auto              imgWidth  = texWidth / 4;
        const auto              imgHeight = texHeight / 3;
        // right
        data.push_back(extractImage(pixels,
                                    2 * imgWidth,
                                    1 * imgHeight,
                                    texWidth,
                                    imgWidth,
                                    imgHeight,
                                    STBI_rgb_alpha));
        // left
        data.push_back(extractImage(pixels,
                                    0 * imgWidth,
                                    1 * imgHeight,
                                    texWidth,
                                    imgWidth,
                                    imgHeight,
                                    STBI_rgb_alpha));
        // top
        data.push_back(extractImage(pixels,
                                    1 * imgWidth,
                                    0 * imgHeight,
                                    texWidth,
                                    imgWidth,
                                    imgHeight,
                                    STBI_rgb_alpha));
        // bottom
        data.push_back(extractImage(pixels,
                                    1 * imgWidth,
                                    2 * imgHeight,
                                    texWidth,
                                    imgWidth,
                                    imgHeight,
                                    STBI_rgb_alpha));
        // front
        data.push_back(extractImage(pixels,
                                    1 * imgWidth,
                                    1 * imgHeight,
                                    texWidth,
                                    imgWidth,
                                    imgHeight,
                                    STBI_rgb_alpha));
        // back
        data.push_back(extractImage(pixels,
                                    3 * imgWidth,
                                    1 * imgHeight,
                                    texWidth,
                                    imgWidth,
                                    imgHeight,
                                    STBI_rgb_alpha));
        auto cubemap = make_shared<Cubemap>(Application::get()._getDevice(),
                                            imgWidth,
                                            imgHeight,
                                            imgWidth * imgHeight * STBI_rgb_alpha,
                                            data);
        for (int i = 0; i < 6; i++) {
            delete[] data[i];
        }
        stbi_image_free(pixels);
        return cubemap;
    }


    void Cubemap::createTextureSampler() {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter    = VK_FILTER_LINEAR;
        samplerInfo.minFilter    = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        // This ensures that the edges between the faces of the cubemap are seamlessly sampled.
        samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // "
        samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // "
        samplerInfo.anisotropyEnable        = VK_TRUE;
        samplerInfo.maxAnisotropy           = device.getDeviceProperties().limits.maxSamplerAnisotropy;
        samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable           = VK_FALSE;
        samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.minLod                  = 0.0f;
        samplerInfo.maxLod                  = 1.0f;
        samplerInfo.mipLodBias              = 0.0f;
        if (vkCreateSampler(device.getDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
            die("failed to create skybox texture sampler");
        }
    }

    unsigned char *Cubemap::extractImage(unsigned char *source,
                                         int            x, int y,
                                         int            srcWidth,
                                         int            w, int h,
                                         int            channels) {
        auto extractedImage = new unsigned char[w * h * channels];
        for (uint32_t row = 0; row < h; ++row) {
            for (uint32_t col = 0; col < w; ++col) {
                for (uint32_t c = 0; c < channels; ++c) {
                    extractedImage[(row * w + col) * channels + c] = source[((y + row) * srcWidth + (x + col)) *
                        channels + c];
                }
            }
        }
        return extractedImage;
    }

}
