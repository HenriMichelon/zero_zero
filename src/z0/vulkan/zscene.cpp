/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>
#include "z0/libraries.h"

module z0.ZScene;

import z0.Device;
import z0.Buffer;
import z0.VulkanImage;

namespace z0 {

    void ZScene::loadTextures(ifstream &stream) {
        auto imageHeaders = vector<ImageHeader>(header.imagesCount);
        auto levelHeaders = vector<vector<MipLevelHeader>>(header.imagesCount);
        auto textureHeaders = vector<TextureHeader>(header.texturesCount);

        uint64_t totalImageSize{0};
        loadImagesAndTexturesHeaders(stream, imageHeaders, levelHeaders, textureHeaders, totalImageSize);

        const auto textureStagingBuffer = Buffer{
            Device::get(),
            totalImageSize,
            1,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        };

        static constexpr size_t BLOCK_SIZE = 64 * 1024;
        auto transferBuffer = vector<char> (BLOCK_SIZE);
        auto transferOffset = VkDeviceSize{0};
        while (stream.read(transferBuffer.data(), BLOCK_SIZE) || stream.gcount() > 0) {
            const auto bytesRead = stream.gcount();
            textureStagingBuffer.writeToBuffer(transferBuffer.data(), bytesRead, transferOffset);
            transferOffset += bytesRead;
        }

        vector<shared_ptr<VulkanImage>> vulkanImages;
        const auto& device = Device::get();
        for (auto textureIndex = 0; textureIndex < header.texturesCount; ++textureIndex) {
            const auto& texture = textureHeaders[textureIndex];
            if (texture.imageIndex != -1) {
                const auto& image = imageHeaders[texture.imageIndex];
                textures.push_back(make_shared<ImageTexture>(
                    make_shared<VulkanImage>(
                       device,
                       image.name,
                       image,
                       levelHeaders[texture.imageIndex],
                       texture,
                       textureStagingBuffer,
                       image.dataOffset)
                ));
            }
        }
    }

}
