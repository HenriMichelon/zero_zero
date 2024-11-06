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
import z0.Mesh;
import z0.Resource;
import z0.Texture;
import z0.Tools;
import z0.VirtualFS;

import z0.VulkanMesh;

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

        // Read the meshes & surfaces headers
        auto meshesHeaders = vector<MeshHeader>(header.meshesCount);
        auto surfaceInfo = vector<vector<SurfaceInfo>> {header.meshesCount};
        auto uvsInfos = vector<vector<vector<DataInfo>>> {header.meshesCount};
        for (auto meshIndex = 0; meshIndex < header.meshesCount; ++meshIndex) {
            stream.read(reinterpret_cast<istream::char_type *>(&meshesHeaders[meshIndex]), sizeof(MeshHeader));
            print(meshesHeaders[meshIndex]);
            surfaceInfo[meshIndex].resize(meshesHeaders[meshIndex].surfacesCount);
            uvsInfos[meshIndex].resize(meshesHeaders[meshIndex].surfacesCount);
            for (auto surfaceIndex = 0; surfaceIndex < meshesHeaders[meshIndex].surfacesCount; ++surfaceIndex) {
                stream.read(reinterpret_cast<istream::char_type *>(&surfaceInfo[meshIndex][surfaceIndex]), sizeof(SurfaceInfo));
                uvsInfos[meshIndex][surfaceIndex].resize(surfaceInfo[meshIndex][surfaceIndex].uvsCount);
                stream.read(reinterpret_cast<istream::char_type *>(uvsInfos[meshIndex][surfaceIndex].data()), sizeof(DataInfo) * surfaceInfo[meshIndex][surfaceIndex].uvsCount);
            }
        }

        // Read the meshes data
        uint32_t count;
        stream.read(reinterpret_cast<istream::char_type *>(&count), sizeof(uint32_t));
        vector<uint32_t> indices(count);
        stream.read(reinterpret_cast<istream::char_type *>(indices.data()), count * sizeof(uint32_t));

        stream.read(reinterpret_cast<istream::char_type *>(&count), sizeof(uint32_t));
        vector<vec3> positions{count};
        stream.read(reinterpret_cast<istream::char_type *>(positions.data()), count * sizeof(vec3));

        stream.read(reinterpret_cast<istream::char_type *>(&count), sizeof(uint32_t));
        vector<vec3> normals{count};
        stream.read(reinterpret_cast<istream::char_type *>(normals.data()), count * sizeof(vec3));

        stream.read(reinterpret_cast<istream::char_type *>(&count), sizeof(uint32_t));
        vector<vec2> uvs{count};
        stream.read(reinterpret_cast<istream::char_type *>(uvs.data()), count * sizeof(vec2));

        stream.read(reinterpret_cast<istream::char_type *>(&count), sizeof(uint32_t));
        vector<vec4> tangents{count};
        stream.read(reinterpret_cast<istream::char_type *>(tangents.data()), count * sizeof(vec4));

        // log(format("{} indices, {} positions, {} normals, {} uvs, {} tangents",
            // indices.size(), positions.size(), normals.size(), uvs.size(), tangents.size()));
        // for(auto& pos : indices) {
            // log(to_string(pos));
        // }

        // Read, upload and create the Image and Texture objets (Vulkan specific)
        loadImagesAndTextures(stream, imageHeaders, levelHeaders, textureHeaders, totalImageSize);

        // Create the Material objects
        map<Resource::id_t, int> materialsTexCoords;
        for (auto materialIndex = 0; materialIndex < header.materialsCount; ++materialIndex) {
            auto& header = materialHeaders[materialIndex];
            auto material = make_shared<StandardMaterial>(header.name);
            auto textureInfo = [&](const TextureInfo& info) {
                auto texInfo = StandardMaterial::TextureInfo {
                    .texture = dynamic_pointer_cast<ImageTexture>(textures[info.textureIndex]),
                    .transform = info.transform,
                };
                materialsTexCoords[material->getId()] = info.uvsIndex;
                return texInfo;
            };
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

        // Create the Mesh, Surface & Vertex objects
        for (auto meshIndex = 0; meshIndex < header.meshesCount; ++meshIndex) {
            auto& header   = meshesHeaders[meshIndex];
            auto  mesh     = make_shared<VulkanMesh>(header.name);
            auto &meshVertices = mesh->getVertices();
            auto &meshIndices  = mesh->getIndices();
            // print(header);
            for (auto surfaceIndex = 0; surfaceIndex < header.surfacesCount; ++surfaceIndex) {
                auto &info = surfaceInfo[meshIndex][surfaceIndex];
                // print(info);
                auto firstIndex = meshIndices.size();
                auto firstVertex  = meshVertices.size();
                auto surface = std::make_shared<Surface>(firstIndex, info.indices.count);
                // Load indices
                meshIndices.reserve(meshIndices.size() + info.indices.count);
                for(auto i = 0; i < info.indices.count; ++i) {
                    meshIndices.push_back(indices[info.indices.first + i]);
                    // log(format("mesh {} surface {} index {}", meshIndex, surfaceIndex, indices[info.indices.first + i]));
                }
                // Load positions
                meshVertices.resize(meshVertices.size() + info.positions.count);
                for(auto i = 0; i < info.positions.count; ++i) {
                    meshVertices[firstVertex + i] = {
                        .position = positions[info.positions.first + i],
                    };
                    // log(format("mesh {} surface {} position {}", meshIndex, surfaceIndex, to_string(positions[info.indices.first + i])));
                }
                // Load normals
                for(auto i = 0; i < info.normals.count; ++i) {
                    meshVertices[firstVertex + i].normal = normals[info.normals.first + i];
                    // log(format("mesh {} surface {} normal  {}", meshIndex, surfaceIndex, to_string(normals[info.normals.first + i])));
                }
                // Load tangents
                for(auto i = 0; i < info.tangents.count; ++i) {
                    meshVertices[firstVertex + i].tangent = tangents[info.tangents.first + i];
                    // log(format("mesh {} surface {} tangents  {}", meshIndex, surfaceIndex, to_string(tangents[info.tangents.first + i])));
                }
                if (info.materialIndex != -1) {
                    // associate material to surface & mesh
                    const auto& material = materials[info.materialIndex];
                    surface->material = material;
                    mesh->_getMaterials().insert(material);
                    // load UVs
                    auto texCoord = 0;
                    if (materialsTexCoords.contains(material->getId())) {
                        texCoord = materialsTexCoords.at(material->getId());
                    }
                    const auto& texCoordInfo = uvsInfos[meshIndex][surfaceIndex][texCoord];
                    for(auto i = 0; i < texCoordInfo.count; i++) {
                        meshVertices[firstVertex + i].uv = uvs[texCoordInfo.first + i];
                        log(format("mesh {} surface {} uvs {} uv {}", meshIndex, surfaceIndex, texCoord, to_string(uvs[texCoordInfo.first + i])));
                    }
                } else {
                    // Mesh have no material, use a default one
                    const auto &material = make_shared<StandardMaterial>();
                    surface->material = material;
                    mesh->_getMaterials().insert(material);
                }
            }
        }

    }

    void ZScene::print(const Header& header) {
        printf("Version : %d\nImages count : %d\nTextures count : %d\nMaterials count : %d\nMeshes count : %d\nHeaders size : %llu\n",
            header.version,
            header.imagesCount,
            header.texturesCount,
            header.materialsCount,
            header.meshesCount,
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

    void ZScene::print(const MeshHeader& header) {
        printf("Name : %s\nSurfaces count : %d\n", header.name, header.surfacesCount);
    }

    void ZScene::print(const SurfaceInfo& header) {
        printf("materialIndex : %d\nindices : %d,%d\npositions : %d,%d\nnormals : %d,%d\ntangents : %d,%d\nuvsCount : %d\n",
            header.materialIndex,
            header.indices.first, header.indices.count,
            header.positions.first, header.positions.count,
            header.normals.first, header.normals.count,
            header.tangents.first, header.tangents.count,
            header.uvsCount);
    }

}
