/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>
#include "z0/libraries.h"

module z0;

import :ZScene;

import :Device;
import :Buffer;
import :VulkanImage;

namespace z0 {

    void ZScene::loadImages(ifstream &stream) {
        auto imageHeader = vector<ImageHeader>(header.imagesCount);
        auto levelHeaders = vector<vector<MipLevelHeader>>(header.imagesCount);

        uint64_t totalImageSize{0};
        loadImagesHeaders(stream, imageHeader, levelHeaders, totalImageSize);

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
        for (auto imageIndex = 0; imageIndex < header.imagesCount; ++imageIndex) {
            images.push_back(make_shared<VulkanImage>(
                device,
                to_string(imageIndex),
                imageHeader[imageIndex],
                levelHeaders[imageIndex],
                textureStagingBuffer,
                imageHeader[imageIndex].dataOffset));
        }
    }

}
