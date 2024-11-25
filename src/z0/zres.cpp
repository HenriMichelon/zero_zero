/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>
#include "z0/libraries.h"

module z0.ZRes;

import z0.Animation;
import z0.AnimationLibrary;
import z0.AnimationPlayer;
import z0.Constants;
import z0.Image;
import z0.Material;
import z0.Mesh;
import z0.MeshInstance;
import z0.Resource;
import z0.Texture;
import z0.Tools;
import z0.VirtualFS;

import z0.VulkanMesh;

namespace z0 {

    shared_ptr<Node> ZRes::load(const string &filename) {
        auto stream = VirtualFS::openStream(filename);
        return load(stream);
    }

    shared_ptr<Node> ZRes::load(ifstream &stream) {
        auto tStart = std::chrono::high_resolution_clock::now();
        auto zscene = make_shared<ZRes>();
        auto rootNode = zscene->loadScene(stream);
        auto last_transcode_time = std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - tStart).count();
        log("ZRes loading time ", to_string(last_transcode_time));
        return rootNode;
    }

    shared_ptr<Node> ZRes::loadScene(ifstream& stream) {
        // Read the file global header
        stream.read(reinterpret_cast<istream::char_type *>(&header), sizeof(header));
        if (header.magic[0] != MAGIC[0] &&
            header.magic[1] != MAGIC[1] &&
            header.magic[2] != MAGIC[2] &&
            header.magic[3] != MAGIC[3]) {
            die("ZScene bad magic");
        }
        if (header.version != VERSION) {
            die("ZScene bad version");
        }
        // print(header);

        // Read the images & mips levels headers
        auto imageHeaders = vector<ImageHeader>(header.imagesCount);
        auto levelHeaders = vector<vector<MipLevelInfo>>(header.imagesCount);
        uint64_t totalImageSize{0};
        for (auto imageIndex = 0; imageIndex < header.imagesCount; ++imageIndex) {
            stream.read(reinterpret_cast<istream::char_type *>(&imageHeaders[imageIndex]), sizeof(ImageHeader));
            // print(imageHeaders[imageIndex]);
            // log(format("{} : {}x{}", string(imageHeaders[imageIndex].name), imageHeaders[imageIndex].width, imageHeaders[imageIndex].height));
            levelHeaders[imageIndex].resize(imageHeaders[imageIndex].mipLevels);
            stream.read(reinterpret_cast<istream::char_type *>(levelHeaders[imageIndex].data()), sizeof(MipLevelInfo) * imageHeaders[imageIndex].mipLevels);
            totalImageSize += imageHeaders[imageIndex].dataSize;
        }

        // Read the textures & materials headers
        auto textureHeaders = vector<TextureHeader>(header.texturesCount);
        if (header.texturesCount > 0) {
            stream.read(reinterpret_cast<istream::char_type *>(textureHeaders.data()), textureHeaders.size() * sizeof(TextureHeader));
        }
        auto materialHeaders = vector<MaterialHeader>(header.materialsCount);
        if (header.materialsCount > 0) {
            stream.read(reinterpret_cast<istream::char_type *>(materialHeaders.data()), materialHeaders.size() * sizeof(MaterialHeader));
        }

        // Read the meshes & surfaces headers
        auto meshesHeaders = vector<MeshHeader>(header.meshesCount);
        auto surfaceInfo = vector<vector<SurfaceInfo>> {header.meshesCount};
        auto uvsInfos = vector<vector<vector<DataInfo>>> {header.meshesCount};
        for (auto meshIndex = 0; meshIndex < header.meshesCount; ++meshIndex) {
            stream.read(reinterpret_cast<istream::char_type *>(&meshesHeaders[meshIndex]), sizeof(MeshHeader));
            // print(meshesHeaders[meshIndex]);
            surfaceInfo[meshIndex].resize(meshesHeaders[meshIndex].surfacesCount);
            uvsInfos[meshIndex].resize(meshesHeaders[meshIndex].surfacesCount);
            for (auto surfaceIndex = 0; surfaceIndex < meshesHeaders[meshIndex].surfacesCount; ++surfaceIndex) {
                stream.read(reinterpret_cast<istream::char_type *>(&surfaceInfo[meshIndex][surfaceIndex]), sizeof(SurfaceInfo));
                // print(surfaceInfo[meshIndex][surfaceIndex]);
                uvsInfos[meshIndex][surfaceIndex].resize(surfaceInfo[meshIndex][surfaceIndex].uvsCount);
                stream.read(reinterpret_cast<istream::char_type *>(uvsInfos[meshIndex][surfaceIndex].data()), sizeof(DataInfo) * uvsInfos[meshIndex][surfaceIndex].size());
            }
        }

        // Read the nodes headers
        auto nodeHeaders = vector<NodeHeader>(header.nodesCount);
        auto childrenIndexes = vector<vector<uint32_t>>(header.nodesCount);
        for (auto nodeIndex = 0; nodeIndex < header.nodesCount; ++nodeIndex) {
            stream.read(reinterpret_cast<istream::char_type *>(&nodeHeaders.at(nodeIndex)), sizeof(NodeHeader));
            childrenIndexes.at(nodeIndex).resize(nodeHeaders.at(nodeIndex).childrenCount);
            stream.read(reinterpret_cast<istream::char_type *>(childrenIndexes.at(nodeIndex).data()), sizeof(uint32_t) * childrenIndexes[nodeIndex].size());
            // log("Node ", nodeHeaders[nodeIndex].name, " ", to_string(nodeHeaders[nodeIndex].childrenCount), "children");
        }

        auto animationHeaders = vector<AnimationHeader>(header.animationsCount);
        auto tracksInfos = vector<vector<TrackInfo>> (header.animationsCount);
        for (auto animationIndex = 0; animationIndex < header.animationsCount; ++animationIndex) {
            stream.read(reinterpret_cast<istream::char_type *>(&animationHeaders.at(animationIndex)), sizeof(AnimationHeader));
            tracksInfos[animationIndex].resize(animationHeaders[animationIndex].tracksCount);
            stream.read(reinterpret_cast<istream::char_type *>(tracksInfos[animationIndex].data()), sizeof(TrackInfo) * tracksInfos[animationIndex].size());
            // log("Animation ", animationHeaders[animationIndex].name, " ", to_string(animationHeaders[animationIndex].tracksCount), "tracks");
        }

        // Skip padding
        // auto pad = vector<uint8_t>(vector<uint8_t>::size_type(32), 0);
        // if (header.headersPadding > 0) {
            // stream.read(reinterpret_cast<istream::char_type *>(pad.data()), header.headersPadding);
        // }

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
        // for(auto& pos : positions) {
        //     log(to_string(pos));
        // }

        // Read the animations data
        auto animationPlayers = map<uint32_t, shared_ptr<AnimationPlayer>>{};
        for (auto animationIndex = 0; animationIndex < header.animationsCount; animationIndex++) {
            auto anim = make_shared<Animation>(static_cast<uint32_t>(animationHeaders[animationIndex].tracksCount), animationHeaders[animationIndex].name);
            for (auto trackIndex = 0; trackIndex < animationHeaders[animationIndex].tracksCount; trackIndex++) {
                auto animationPlayer = shared_ptr<AnimationPlayer>{};
                auto& trackInfo = tracksInfos[animationIndex][trackIndex];
                auto nodeIndex = trackInfo.nodeIndex;
                if (animationPlayers.contains(nodeIndex)) {
                    animationPlayer = animationPlayers[nodeIndex];
                } else {
                    animationPlayer = make_shared<AnimationPlayer>(); // node association is made later
                    animationPlayer->add("", make_shared<AnimationLibrary>());
                    animationPlayers[nodeIndex] = animationPlayer;
                }
                animationPlayer->getLibrary()->add(anim->getName(), anim);
                animationPlayer->setCurrentAnimation(anim->getName());
                auto& track = anim->getTrack(trackIndex);
                track.type = static_cast<AnimationType>(trackInfo.type);
                track.interpolation = static_cast<AnimationInterpolation>(trackInfo.interpolation);
                track.keyTime.resize(trackInfo.keysCount);
                stream.read(reinterpret_cast<istream::char_type *>(track.keyTime.data()), trackInfo.keysCount * sizeof(float));
                track.duration = track.keyTime.back() + track.keyTime.front();
                track.keyValue.resize(trackInfo.keysCount);
                stream.read(reinterpret_cast<istream::char_type *>(track.keyValue.data()), trackInfo.keysCount * sizeof(variant<vec3, quat>));
            }
        }

        // Read, upload and create the Image and Texture objets (Vulkan specific)
        if (header.imagesCount > 0) {
            loadImagesAndTextures(stream, imageHeaders, levelHeaders, textureHeaders, totalImageSize);
        }

        // Create the Material objects
        vector<shared_ptr<Material>> materials{static_cast<vector<shared_ptr<Material>>::size_type>(header.materialsCount)};
        map<Resource::id_t, int> materialsTexCoords;
        for (auto materialIndex = 0; materialIndex < header.materialsCount; ++materialIndex) {
            auto& header = materialHeaders.at(materialIndex);
            auto material = make_shared<StandardMaterial>(header.name);
            auto textureInfo = [&](const TextureInfo& info) {
                auto texInfo = StandardMaterial::TextureInfo {
                    .texture = info.textureIndex == -1 ? nullptr : dynamic_pointer_cast<ImageTexture>(textures.at(info.textureIndex)),
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
            material->setNormalScale(header.normalScale);
            materials.at(materialIndex) = material;
        }

        // Create the Mesh, Surface & Vertex objects
        vector<shared_ptr<Mesh>> meshes{static_cast<vector<shared_ptr<Mesh>>::size_type>(header.meshesCount)};
        for (auto meshIndex = 0; meshIndex < header.meshesCount; ++meshIndex) {
            auto& header   = meshesHeaders.at(meshIndex);
            auto  mesh     = make_shared<VulkanMesh>(header.name);
            auto &meshVertices = mesh->getVertices();
            auto &meshIndices  = mesh->getIndices();
            // print(header);
            for (auto surfaceIndex = 0; surfaceIndex < header.surfacesCount; ++surfaceIndex) {
                auto &info = surfaceInfo.at(meshIndex)[surfaceIndex];
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
                    // log(format("mesh {} surface {} position {}", meshIndex, surfaceIndex, to_string(positions[info.positions.first + i])));
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
                    const auto& texCoordInfo = uvsInfos.at(meshIndex)[surfaceIndex][texCoord];
                    for(auto i = 0; i < texCoordInfo.count; i++) {
                        meshVertices[firstVertex + i].uv = uvs[texCoordInfo.first + i];
                        // log(format("mesh {} surface {} uvs {} uv {}", meshIndex, surfaceIndex, texCoord, to_string(uvs[texCoordInfo.first + i])));
                    }
                } else {
                    // Mesh have no material, use a default one
                    const auto &material = make_shared<StandardMaterial>();
                    surface->material = material;
                    mesh->_getMaterials().insert(material);
                }
                // calculate missing tangents
                if (info.tangents.count == 0) {
                    for (auto i = 0; i < meshIndices.size(); i += 3) {
                        auto &vertex1  = meshVertices[meshIndices[i]];
                        auto &vertex2  = meshVertices[meshIndices[i + 1]];
                        auto &vertex3  = meshVertices[meshIndices[i + 2]];
                        vec3  edge1    = vertex2.position - vertex1.position;
                        vec3  edge2    = vertex3.position - vertex1.position;
                        vec2  deltaUV1 = vertex2.uv - vertex1.uv;
                        vec2  deltaUV2 = vertex3.uv - vertex1.uv;

                        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
                        vec3  tangent{
                            f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
                            f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
                            f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z),
                        };
                        vertex1.tangent = vec4(tangent, 1.0);
                        vertex2.tangent = vec4(tangent, 1.0);
                        vertex3.tangent = vec4(tangent, 1.0);
                    }
                }
                mesh->getSurfaces().push_back(surface);
            }
            mesh->buildModel();
            meshes[meshIndex] = mesh;
        }

        // Create the Node objects
        vector<shared_ptr<Node>> nodes{static_cast<vector<shared_ptr<Node>>::size_type>(header.nodesCount)};
        for (auto nodeIndex = 0; nodeIndex < header.nodesCount; ++nodeIndex) {
            shared_ptr<Node> newNode;
            string           name{nodeHeaders.at(nodeIndex).name};
            // find if the node has a mesh, and if it does hook it to the mesh pointer and allocate it with the
            // MeshInstance class
            if (nodeHeaders.at(nodeIndex).meshIndex != -1) {
                auto mesh = meshes[nodeHeaders.at(nodeIndex).meshIndex];
                newNode = std::make_shared<MeshInstance>(mesh, name);
            } else {
                newNode = std::make_shared<Node>(name);
            }
            newNode->_setTransform(nodeHeaders.at(nodeIndex).transform);
            newNode->_updateTransform(mat4{1.0f});
            nodes[nodeIndex] = newNode;
        }

        for (auto animationIndex = 0; animationIndex < header.animationsCount; animationIndex++) {
            for (auto trackIndex = 0; trackIndex < animationHeaders[animationIndex].tracksCount; trackIndex++) {
                auto nodeIndex = tracksInfos[animationIndex][trackIndex].nodeIndex;
                auto& player = animationPlayers[nodeIndex];
                if (!player->getNode()) {
                    auto& node = nodes[nodeIndex];
                    player->setNode(node);
                    node->addChild(player);
                }
            }
        }

        // Build the scene tree
        for (auto nodeIndex = 0; nodeIndex < header.nodesCount; ++nodeIndex) {
            auto& sceneNode = nodes[nodeIndex];
            for (auto i = 0; i < nodeHeaders.at(nodeIndex).childrenCount; i++) {
                sceneNode->addChild(nodes[childrenIndexes.at(nodeIndex)[i]]);
            }
        }

        // find the top nodes, with no parents
        auto rootNode = make_shared<Node>("ZScene");
        for (auto &node : nodes) {
            if (node->getParent() == nullptr) {
                rootNode->addChild(node);
            }
        }

        return rootNode;
    }

    void ZRes::print(const Header& header) {
        printf("Version : %d\nImages count : %d\nTextures count : %d\nMaterials count : %d\nMeshes count : %d\nNodes count : %d\nAnimations count : %d\nHeaders size : %llu\n",
            header.version,
            header.imagesCount,
            header.texturesCount,
            header.materialsCount,
            header.meshesCount,
            header.nodesCount,
            header.animationsCount,
            header.headersSize);
    }
    void ZRes::print(const ImageHeader& header) {
        printf("Name : %s\nFormat : %d\nWidth : %d\nHeight : %d\nLevels : %d\ndataOffset : %llu\ndataSize : %llu\n",
            header.name,
            header.format,
            header.width,
            header.height,
            header.mipLevels,
            header.dataOffset,
            header.dataSize);
    }
    void ZRes::print(const MipLevelInfo& header) {
        printf("Offset : %llu\nSize: %llu\n", header.offset, header.size);
    }

    void ZRes::print(const TextureHeader& header) {
        printf("imageIndex : %d\nminFilter : %d\nmagFilter : %d\nsampleAddressModeU : %d\nsampleAddressModeV : %d\n",
            header.imageIndex,
            header.minFilter,
            header.magFilter,
            header.samplerAddressModeU,
            header.samplerAddressModeV);
    }

    void ZRes::print(const MaterialHeader& header) {
        printf("Name : %s\n", header.name);
    }

    void ZRes::print(const MeshHeader& header) {
        printf("Name : %s\nSurfaces count : %d\n", header.name, header.surfacesCount);
    }

    void ZRes::print(const DataInfo& header) {
        printf("First : %d\nCount : %d\n", header.first, header.count);
    }

    void ZRes::print(const SurfaceInfo& header) {
        printf("materialIndex : %d\nindices : %d,%d\npositions : %d,%d\nnormals : %d,%d\ntangents : %d,%d\nuvsCount : %d\n",
            header.materialIndex,
            header.indices.first, header.indices.count,
            header.positions.first, header.positions.count,
            header.normals.first, header.normals.count,
            header.tangents.first, header.tangents.count,
            header.uvsCount);
    }

}
