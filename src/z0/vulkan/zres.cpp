/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"
#include "z0/vulkan.h"

module z0.ZRes;

import z0.Tools;

import z0.vulkan.Buffer;
import z0.vulkan.Device;
import z0.vulkan.Image;

namespace z0 {

    void ZRes::loadImagesAndTextures(
        const Device& device,
        ifstream &stream,
        const vector<ImageHeader>& imageHeaders,
        const vector<vector<MipLevelInfo>>&levelHeaders,
        const vector<TextureHeader>& textureHeaders,
        const uint64_t totalImageSize) {

        // Upload all images into VRAM using one big staging buffer
        const auto command = device.beginOneTimeCommandBuffer();
        const auto& textureStagingBuffer = device.createOneTimeBuffer(
            command,
            totalImageSize,
            1,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT
        );
        static constexpr size_t BLOCK_SIZE = 64 * 1024;
        auto transferBuffer = vector<char> (BLOCK_SIZE);
        auto transferOffset = VkDeviceSize{0};
        while (stream.read(transferBuffer.data(), BLOCK_SIZE) || stream.gcount() > 0) {
            const auto bytesRead = stream.gcount();
            textureStagingBuffer.writeToBuffer(transferBuffer.data(), bytesRead, transferOffset);
            transferOffset += bytesRead;
        }
        //printf("%llu bytes read\n", transferOffset);

        // Create all images from this staging buffer
        vector<shared_ptr<VulkanImage>> vulkanImages;
        for (auto textureIndex = 0; textureIndex < header.texturesCount; ++textureIndex) {
            const auto& texture = textureHeaders[textureIndex];
            if (texture.imageIndex != -1) {
                const auto& image = imageHeaders[texture.imageIndex];
                //log("Loaded image ", image.name, to_string(image.width), "x", to_string(image.height), to_string(image.format));
                // print(image);
                textures.push_back(make_shared<ImageTexture>(
                    make_shared<VulkanImage>(
                       device,
                       command.commandBuffer,
                       image.name,
                       image,
                       levelHeaders[texture.imageIndex],
                       texture,
                       textureStagingBuffer,
                       image.dataOffset)
                ));
            }
        }
        device.endOneTimeCommandBuffer(command);
    }

}
