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

import z0.Image;
import z0.Texture;
import z0.Tools;
import z0.VirtualFS;

namespace z0 {

    shared_ptr<ZScene> ZScene::load(const string &filename) {
        auto stream = VirtualFS::openStream(filename);
        return load(stream);
    }

    shared_ptr<ZScene> ZScene::load(ifstream &stream) {
        auto tStart = std::chrono::high_resolution_clock::now();
        auto zscene = make_shared<ZScene>();
        zscene->loadHeader(stream);
        zscene->loadTextures(stream);
        auto last_transcode_time = std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - tStart).count();
        log("ZScene loading time ", to_string(last_transcode_time));
        return zscene;
    }

    void ZScene::loadHeader(ifstream& stream) {
        stream.read(reinterpret_cast<istream::char_type *>(&header), sizeof(header));
        if (header.magic[0] != Header::MAGIC[0] &&
            header.magic[1] != Header::MAGIC[1] &&
            header.magic[2] != Header::MAGIC[2] &&
            header.magic[3] != Header::MAGIC[3]) {
            die("ZScene bad magic");
            }
        if (header.version != Header::VERSION) {
            die("ZScene bad version");
        }
    }

    void ZScene::loadImagesAndTexturesHeaders(
            ifstream& stream,
            vector<ImageHeader>&imageHeader,
            vector<vector<MipLevelHeader>>&levelHeaders,
            vector<TextureHeader>&textureHeaders,
            uint64_t& totalImageSize) const {
        for (auto imageIndex = 0; imageIndex < header.imagesCount; ++imageIndex) {
            stream.read(reinterpret_cast<istream::char_type *>(&imageHeader[imageIndex]), sizeof(ImageHeader));
            levelHeaders[imageIndex].resize(imageHeader[imageIndex].mipLevels);
            stream.read(reinterpret_cast<istream::char_type *>(levelHeaders[imageIndex].data()), sizeof(MipLevelHeader) * imageHeader[imageIndex].mipLevels);
            totalImageSize += imageHeader[imageIndex].dataSize;
        }
        for (auto textureIndex = 0; textureIndex < header.texturesCount; ++textureIndex) {
            stream.read(reinterpret_cast<istream::char_type *>(&textureHeaders[textureIndex]), sizeof(TextureHeader));
        }
    }

    void ZScene::print(const Header& header) {
        printf("Version : %d\nImages count : %d\nTextures count : %d\nHeaders size : %llu\n",
            header.version,
            header.imagesCount,
            header.texturesCount,
            header.headersSize);
    }

    void ZScene::print(const ImageHeader& header) {
        printf("Name : %s\nFormat : %d\nWidth : %d\nHeight : %d\nLevels : %d\ndataOffset : %llu\ndataSize : %llu\n",
            header.name,
            header.format,
            header.width,
            header.height,
            header.mipLevels,
            header.dataOffset,
            header.dataSize);
    }

    void ZScene::print(const MipLevelHeader& header) {
        printf("Offset : %llu\nSize: %llu\n", header.offset, header.size);
    }

    void ZScene::print(const TextureHeader& header) {
        printf("imageIndex : %d\nminFilter : %d\nmagFilter : %d\nsampleAddressModeU : %d\nsampleAddressModeV : %d\n",
            header.imageIndex,
            header.minFilter,
            header.magFilter,
            header.samplerAddressModeU,
            header.samplerAddressModeV);
    }


}
