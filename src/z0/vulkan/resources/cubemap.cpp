module;
#include <cassert>
#include <volk.h>
#include "z0/libraries.h"

module z0;

import :Application;
import :Cubemap;
import :Image;

import :Device;
import :Buffer;
import :VulkanCubemap;
import :VulkanImage;
import :IBLPipeline;

namespace z0 {

    shared_ptr<Cubemap> Cubemap::create(
        const uint32_t                 width,
        const uint32_t                 height,
        const uint32_t                 imageSize,
        const vector<unsigned char *> &data,
        const string &                 name) {
        return make_shared<VulkanCubemap>(Application::get()._getDevice(), width, height, imageSize, data, name);
    }

    VulkanCubemap::VulkanCubemap(
        const Device &                  device,
        const uint32_t                  width,
        const uint32_t                  height,
        const uint32_t                  imageSize,
        const vector<unsigned char *> & data,
        const string &                  name):
        Cubemap{width, height, TYPE_STANDARD, name}, device{device}, textureFormat{VK_FORMAT_R8G8B8A8_SRGB} {
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
        device.createImage(width,
                           height,
                           1,
                           VK_SAMPLE_COUNT_1_BIT,
                           textureFormat,
                           VK_IMAGE_TILING_OPTIMAL,
                           VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                           VK_IMAGE_USAGE_SAMPLED_BIT,
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
        const VkCommandBuffer commandBuffer = device.beginOneTimeCommandBuffer();
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
        device.endOneTimeCommandBuffer(commandBuffer);

        textureImageView = device.createImageView(textureImage,
                                                  textureFormat,
                                                  VK_IMAGE_ASPECT_COLOR_BIT,
                                                  1,
                                                  VK_IMAGE_VIEW_TYPE_CUBE);
        createTextureSampler();
    }

    VulkanCubemap::VulkanCubemap(const Device &  device,
                     const uint32_t  width,
                     const uint32_t  height,
                     const uint32_t  levels,
                     const VkFormat  format,
                     const string &  name):
        Cubemap(width, height, TYPE_ENVIRONMENT, name), device{device}, textureFormat{format}  {
        // Create image in GPU memory, one image in memory for the 6 images of the cubemap
        device.createImage(width,
                           height,
                           levels,
                           VK_SAMPLE_COUNT_1_BIT,
                           textureFormat,
                           VK_IMAGE_TILING_OPTIMAL,
                           VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                           VK_IMAGE_USAGE_SAMPLED_BIT |
                           VK_IMAGE_USAGE_STORAGE_BIT, // for compute shaders (convert from HDRi)
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                           textureImage,
                           textureImageMemory,
                           VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
                           6);
        textureImageView = device.createImageView(textureImage,
                                                  textureFormat,
                                                  VK_IMAGE_ASPECT_COLOR_BIT,
                                                  levels,
                                                  VK_IMAGE_VIEW_TYPE_CUBE);
        createTextureSampler();
    }

    VulkanCubemap::~VulkanCubemap() {
        vkDestroySampler(device.getDevice(), textureSampler, nullptr);
        vkDestroyImageView(device.getDevice(), textureImageView, nullptr);
        vkDestroyImage(device.getDevice(), textureImage, nullptr);
        vkFreeMemory(device.getDevice(), textureImageMemory, nullptr);
    }

    void VulkanCubemap::createTextureSampler() {
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

    EnvironmentCubemap::EnvironmentCubemap(const string &  name):
        Cubemap{ENVIRONMENT_MAP_SIZE, ENVIRONMENT_MAP_SIZE, TYPE_ENVIRONMENT, name} {
        auto& device = Application::get()._getDevice();
        specularCubemap = make_shared<VulkanCubemap>(device,
            ENVIRONMENT_MAP_SIZE,
            ENVIRONMENT_MAP_SIZE,
            ENVIRONMENT_MAP_MIPMAP_LEVELS);
        irradianceCubemap = make_shared<VulkanCubemap>(device,
           IRRADIANCE_MAP_SIZE,
           IRRADIANCE_MAP_SIZE);
        brdfLut = make_shared<VulkanImage>(device,
            BRDFLUT_SIZE,
            BRDFLUT_SIZE,
            1,
            VK_FORMAT_R16G16_SFLOAT,
            1,
            VK_IMAGE_USAGE_STORAGE_BIT,
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            VK_FILTER_LINEAR,
            VK_FALSE
        );
    }

    shared_ptr<EnvironmentCubemap> EnvironmentCubemap::loadFromHDRi(const string &filename) {
        auto& device = Application::get()._getDevice();
        auto envCubemap = make_shared<EnvironmentCubemap>();
        const auto unfilteredCubemap = make_shared<VulkanCubemap>(device,
            ENVIRONMENT_MAP_SIZE,
            ENVIRONMENT_MAP_SIZE
        );
        const auto &vkSpecular = reinterpret_pointer_cast<VulkanCubemap>(envCubemap->specularCubemap);
        const auto &vkIrradiance = reinterpret_pointer_cast<VulkanCubemap>(envCubemap->irradianceCubemap);
        const auto &vkBRDF = reinterpret_pointer_cast<VulkanImage>(envCubemap->brdfLut);
        const auto iblPipeline = IBLPipeline{device};
        iblPipeline.convert(reinterpret_pointer_cast<VulkanImage>(Image::loadFromFile(filename)), unfilteredCubemap);
        iblPipeline.preComputeSpecular(unfilteredCubemap, vkSpecular);
        iblPipeline.preComputeIrradiance(vkSpecular, vkIrradiance);
        iblPipeline.preComputeBRDF(vkBRDF);
        return envCubemap;
    }

}