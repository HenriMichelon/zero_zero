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

import z0.Constants;
import z0.Image;
import z0.Material;
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
        zscene->loadScene(stream);
        auto last_transcode_time = std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - tStart).count();
        log("ZScene loading time ", to_string(last_transcode_time));
        return zscene;
    }

    void ZScene::loadScene(ifstream& stream) {
        // Read the file global header
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

        // Read the images & mips levels headers
        auto imageHeaders = vector<ImageHeader>(header.imagesCount);
        auto levelHeaders = vector<vector<MipLevelHeader>>(header.imagesCount);
        uint64_t totalImageSize{0};
        for (auto imageIndex = 0; imageIndex < header.imagesCount; ++imageIndex) {
                stream.read(reinterpret_cast<istream::char_type *>(&imageHeaders[imageIndex]), sizeof(ImageHeader));
                levelHeaders[imageIndex].resize(imageHeaders[imageIndex].mipLevels);
                stream.read(reinterpret_cast<istream::char_type *>(levelHeaders[imageIndex].data()), sizeof(MipLevelHeader) * imageHeaders[imageIndex].mipLevels);
                totalImageSize += imageHeaders[imageIndex].dataSize;
        }

        // Read the textures & materials headers
        auto textureHeaders = vector<TextureHeader>(header.texturesCount);
        stream.read(reinterpret_cast<istream::char_type *>(textureHeaders.data()), textureHeaders.size() * sizeof(TextureHeader));
        auto materialHeaders = vector<MaterialHeader>(header.materialsCount);
        stream.read(reinterpret_cast<istream::char_type *>(materialHeaders.data()), materialHeaders.size() * sizeof(MaterialHeader));

        // Create the images and textures objets (Vulkan specific)
        loadImagesAndTextures(stream, imageHeaders, levelHeaders, textureHeaders, totalImageSize);

        // Create the materials objects
        auto textureInfo = [this](const TextureInfo& info) {
            auto texInfo = StandardMaterial::TextureInfo {
                .texture = dynamic_pointer_cast<ImageTexture>(textures[info.textureIndex]),
                .transform = info.transform,
            };
            return texInfo;
        };
        for (auto materialIndex = 0; materialIndex < header.materialsCount; ++materialIndex) {
            auto& header = materialHeaders[materialIndex];
            auto material = make_shared<StandardMaterial>(header.name);
            material->setCullMode(static_cast<CullMode>(header.cullMode));
            material->setTransparency(static_cast<Transparency>(header.transparency));
            material->setAlphaScissor(header.alphaScissor);
            material->setAlbedoColor(header.albedoColor);
            material->setAlbedoTexture(textureInfo(header.albedoTexture));
            material->setMetallicFactor(header.metallicFactor);
            material->setMetallicTexture(textureInfo(header.metallicTexture));
            material->setRoughnessFactor(header.roughnessFactor);
            material->setRoughnessTexture(textureInfo(header.roughnessTexture));
            material->setEmissiveFactor(header.emissiveFactor);
            material->setEmissiveStrength(header.emissiveStrength);
            material->setEmissiveTexture(textureInfo(header.emissiveTexture));
            material->setNormalTexture(textureInfo(header.normalTexture));
            material->setNormaleScale(header.normalScale);
            materials.push_back(material);
        }
    }

    void ZScene::print(const Header& header) {
        printf("Version : %d\nImages count : %d\nTextures count : %d\nMaterials count : %d\nHeaders size : %llu\n",
            header.version,
            header.imagesCount,
            header.texturesCount,
            header.materialsCount,
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

    void ZScene::print(const MaterialHeader& header) {
        printf("Name : %s\n", header.name);
    }


}
