/*
 * Copyright (c) 2024 Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
#include <cxxopts.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <glm/gtx/quaternion.hpp>
#include <volk.h>
#include <z0/z0.h>
using namespace std;
namespace fs = std::filesystem;

import miplevel;
import converter;
import image;

int main(const int argc, char** argv) {
    cxxopts::Options options("gltl2zscene", "Create a ZScene file from a glTF file");
    options.add_options()
        ("f", "Compression format for color textures : bc1 bc2 bc3 bc4 bc4s bc5 bc5s bc7, default : bc7", cxxopts::value<string>())
        ("t", "Number of threads for transcoding, default : auto", cxxopts::value<int>())
        ("v", "Verbose mode")
        ("input", "The binary glTF file to read", cxxopts::value<string>())
        ("output","The ZScene file to create", cxxopts::value<string>());
    options.parse_positional({"input", "output"});
    const auto result = options.parse(argc, argv);
    if (result.count("input") != 1 || result.count("output") != 1) {
        cerr << "usage: gltl2zscene input.glb output.zscene\n";
        return EXIT_FAILURE;
    }

    // -v
    auto verbose = result.count("v") > 0;

    /// -f
    auto formatName = string{"bc7"};
    if (result.count("f") == 1) {
        formatName = result["f"].as<string>();
    }
    if (!formats.contains(formatName)) {
        cerr << "Unsupported compression format " << formatName << endl;
        return EXIT_FAILURE;
    }
    if (verbose) { cout << "Using compression format " << formatName << " for color images" << endl; }

    // -t
    auto maxThreads = 0;
    if (result.count("t") == 1) {
        maxThreads = result["t"].as<int>();
    }
    if (maxThreads <= 0) {
        maxThreads = std::max<int>(4, thread::hardware_concurrency() / 2);
    }
    if (verbose) { cout << "Using " << maxThreads << " threads for transcoding" << endl; }

    // positionals args
    const auto &inputFilename  = fs::path(result["input"].as<string>());
    const auto &outputFilename = fs::path(result["output"].as<string>());
    if (!fs::exists(inputFilename)) {
        cerr << "Input file " << inputFilename.string() <<  " not found";
        return EXIT_FAILURE;
    }

    // Open the glTF file
    if (verbose) { cout << "Opening file " << inputFilename << endl; }
    constexpr auto gltfOptions =
        fastgltf::Options::DontRequireValidAssetMember |
        fastgltf::Options::AllowDouble |
        fastgltf::Options::LoadGLBBuffers |
        fastgltf::Options::LoadExternalBuffers;
    auto gltfFile = fastgltf::GltfDataBuffer::FromPath(inputFilename);
    if (auto error = gltfFile.error(); error != fastgltf::Error::None) {
        cerr << getErrorMessage(error) << endl;
        return EXIT_FAILURE;
    }

    // Read the glTF JSON file or GLB headers
    fastgltf::Parser parser{fastgltf::Extensions::KHR_materials_specular |
                            fastgltf::Extensions::KHR_texture_transform |
                            fastgltf::Extensions::KHR_materials_emissive_strength};
    auto asset = parser.loadGltf(gltfFile.get(), inputFilename.parent_path(), gltfOptions);
    if (auto error = asset.error(); error != fastgltf::Error::None) {
        cerr << getErrorMessage(error) << endl;
        return EXIT_FAILURE;
    }
    fastgltf::Asset gltf = std::move(asset.get());

    // First check for images usage to select the destination format
    auto imagesFormat = vector<ImageFormat>(gltf.images.size());
    for(const auto& material : gltf.materials) {
        if (material.pbrData.baseColorTexture.has_value() && gltf.textures[material.pbrData.baseColorTexture.value().textureIndex].imageIndex.has_value()) {
            imagesFormat[gltf.textures[material.pbrData.baseColorTexture.value().textureIndex].imageIndex.value()] =
                { formatName, formats.at(formatName).formatSRGB };
        }
        if (material.pbrData.metallicRoughnessTexture.has_value()&& gltf.textures[material.pbrData.metallicRoughnessTexture.value().textureIndex].imageIndex.has_value()) {
            imagesFormat[gltf.textures[material.pbrData.metallicRoughnessTexture.value().textureIndex].imageIndex.value()] =
                { "bc7", formats.at("bc7").format };
        }
        if (material.normalTexture.has_value()&& gltf.textures[material.normalTexture.value().textureIndex].imageIndex.has_value()) {
            imagesFormat[gltf.textures[material.normalTexture.value().textureIndex].imageIndex.value()] =
                { "bc7", formats.at("bc7").format };
        }
        if (material.emissiveTexture.has_value() && gltf.textures[material.emissiveTexture.value().textureIndex].imageIndex.has_value()) {
            imagesFormat[gltf.textures[material.emissiveTexture.value().textureIndex].imageIndex.value()] =
                { formatName, formats.at(formatName).formatSRGB };
        }
    }

    // Load & transcode all the images
    if (verbose) { cout << "Transcoding images...\n"; }
    auto outMipLevels = vector<vector<MipLevel>>(gltf.images.size());
    try {
        auto tStart = std::chrono::high_resolution_clock::now();
        for (auto imageIndex = 0; imageIndex < gltf.images.size(); imageIndex++) {
            equeueImageLoading(
                gltf,
                gltf.images[imageIndex],
                outMipLevels[imageIndex],
                imagesFormat.at(imageIndex).name,
                verbose);
        }
        startImagesLoading(maxThreads);
        if (verbose) {
            auto last_transcode_time = std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - tStart).count();
            cout << "Total transcoding time for "  << gltf.images.size() << " images : " <<  last_transcode_time << "ms" << endl;
        }
    } catch (string& e) {
        cerr << e << endl;
        return EXIT_FAILURE;
    }

    // Initialize the destination file headers
    if (verbose) { cout << "Creating destination file headers...\n"; }
    auto header = z0::ZRes::Header {
        .version = 1,
        .imagesCount = static_cast<uint32_t>(gltf.images.size()),
        .texturesCount = static_cast<uint32_t>(gltf.textures.size()),
        .materialsCount = static_cast<uint32_t>(gltf.materials.size()),
        .meshesCount = static_cast<uint32_t>(gltf.meshes.size()),
        .nodesCount = static_cast<uint32_t>(gltf.nodes.size()),
        .headersSize = sizeof(z0::ZRes::Header),
    };
    header.magic[0] = z0::ZRes::MAGIC[0];
    header.magic[1] = z0::ZRes::MAGIC[1];
    header.magic[2] = z0::ZRes::MAGIC[2];
    header.magic[3] = z0::ZRes::MAGIC[3];

    auto copyName = [](string name, char* dest, auto index) {
        if (name.empty()) { name = to_string(index); }
        memset(dest, 0, z0::ZRes::NAME_SIZE);
        strncpy(dest, name.c_str(), z0::ZRes::NAME_SIZE-1);
        dest[z0::ZRes::NAME_SIZE - 1] = '\0';
    };

    // Fill the images header
    auto imageHeaders = vector<z0::ZRes::ImageHeader> {gltf.images.size()};
    auto mipHeaders = vector<vector<z0::ZRes::MipLevelInfo>>{gltf.images.size()};
    uint64_t dataOffset = 0;
    for(auto imageIndex = 0; imageIndex < gltf.images.size(); imageIndex++) {
        copyName(gltf.images[imageIndex].name.data(), imageHeaders[imageIndex].name, imageIndex);
        imageHeaders[imageIndex].format = imagesFormat.at(imageIndex).format;

        // Set header for each mip level of the image
        uint64_t mipOffset{0};
        mipHeaders[imageIndex].resize(outMipLevels[imageIndex].size());
        for(auto mipLevel = 0; mipLevel < outMipLevels[imageIndex].size(); ++mipLevel) {
            mipHeaders[imageIndex][mipLevel].offset = mipOffset;
            mipHeaders[imageIndex][mipLevel].size = outMipLevels[imageIndex][mipLevel].data->size();
            // mipHeaders[imageIndex][mipLevel].padding = calculatePadding(mipOffset, imageHeaders[imageIndex].format);
            mipOffset += mipHeaders[imageIndex][mipLevel].size; // + mipHeaders[imageIndex][mipLevel].padding;
        }

        // Set header for one image
        imageHeaders[imageIndex].width  = outMipLevels[imageIndex][0].width;
        imageHeaders[imageIndex].height  = outMipLevels[imageIndex][0].height;
        imageHeaders[imageIndex].mipLevels  = static_cast<uint32_t>(outMipLevels[imageIndex].size());
        imageHeaders[imageIndex].dataOffset = dataOffset;
        imageHeaders[imageIndex].dataSize = mipOffset;
        // imageHeaders[imageIndex].padding = calculatePadding(mipOffset, imageHeaders[imageIndex].format);
        dataOffset += mipOffset; // + imageHeaders[imageIndex].padding;
        // ZScene::print(imageHeaders[imageIndex]);
        header.headersSize += sizeof(z0::ZRes::ImageHeader) + sizeof(z0::ZRes::MipLevelInfo) * outMipLevels[imageIndex].size();
    }

    // Fill textures headers
    auto textureHeaders = vector<z0::ZRes::TextureHeader> {gltf.textures.size()};
    for(auto textureIndex = 0; textureIndex < gltf.textures.size(); textureIndex++) {
        const auto& texture = gltf.textures[textureIndex];
        textureHeaders[textureIndex].imageIndex = texture.imageIndex.has_value() ? static_cast<int32_t>(texture.imageIndex.value()) : -1;
        textureHeaders[textureIndex].minFilter = VK_FILTER_LINEAR;
        textureHeaders[textureIndex].magFilter = VK_FILTER_LINEAR;
        textureHeaders[textureIndex].samplerAddressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        textureHeaders[textureIndex].samplerAddressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        if (texture.samplerIndex.has_value()) {
            const auto& sampler = gltf.samplers[texture.samplerIndex.value()];
            if (sampler.magFilter.has_value()) {
                textureHeaders[textureIndex].magFilter = sampler.magFilter.value() == fastgltf::Filter::Linear ? VK_FILTER_LINEAR : VK_FILTER_LINEAR;
            }
            if (sampler.minFilter.has_value()) {
                const auto v = sampler.minFilter.value();
                textureHeaders[textureIndex].minFilter = (v==fastgltf::Filter::Linear || v==fastgltf::Filter::LinearMipMapLinear || v==fastgltf::Filter::LinearMipMapNearest) ?
                    VK_FILTER_LINEAR : VK_FILTER_LINEAR;
            }
            textureHeaders[textureIndex].samplerAddressModeU = sampler.wrapS ==
                fastgltf::Wrap::ClampToEdge ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE :
                    sampler.wrapS == fastgltf::Wrap::Repeat ? VK_SAMPLER_ADDRESS_MODE_REPEAT :
                        VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            textureHeaders[textureIndex].samplerAddressModeV = sampler.wrapT ==
                fastgltf::Wrap::ClampToEdge ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE :
                    sampler.wrapT == fastgltf::Wrap::Repeat ? VK_SAMPLER_ADDRESS_MODE_REPEAT :
                        VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        }
        header.headersSize += sizeof(z0::ZRes::TextureHeader);
    }

    // Fill materials headers
    auto materialHeaders = vector<z0::ZRes::MaterialHeader> {gltf.materials.size()};
    for(auto materialIndex = 0; materialIndex < gltf.materials.size(); materialIndex++) {
        const auto& material = gltf.materials[materialIndex];
        copyName(material.name.data(), materialHeaders[materialIndex].name, materialIndex);

        materialHeaders[materialIndex].albedoColor = {
            material.pbrData.baseColorFactor[0],
            material.pbrData.baseColorFactor[1],
            material.pbrData.baseColorFactor[2],
            material.pbrData.baseColorFactor[3],
        };
        materialHeaders[materialIndex].metallicFactor = material.pbrData.metallicFactor;
        materialHeaders[materialIndex].roughnessFactor = material.pbrData.roughnessFactor;
        materialHeaders[materialIndex].emissiveFactor =glm::vec3{material.emissiveFactor[0], material.emissiveFactor[1], material.emissiveFactor[2]};
        materialHeaders[materialIndex].emissiveStrength = material.emissiveStrength;
        switch (material.alphaMode) {
        case fastgltf::AlphaMode::Blend:
            materialHeaders[materialIndex].transparency = static_cast<uint32_t>(z0::Transparency::ALPHA);
            break;
        case fastgltf::AlphaMode::Mask:
            materialHeaders[materialIndex].transparency =  static_cast<uint32_t>(z0::Transparency::SCISSOR);
            materialHeaders[materialIndex].alphaScissor = material.alphaCutoff;
            break;
        default:
            break;
        }
        materialHeaders[materialIndex].cullMode =  static_cast<uint32_t>(material.doubleSided ? z0::CullMode::DISABLED : z0::CullMode::BACK);
        auto textureInfo = [](const fastgltf::TextureInfo& sourceTextureInfo) {
            glm::mat3 translation;
            glm::mat3 scale;
            glm::mat3 rotation;
            const auto& transform = sourceTextureInfo.transform;
            if (transform != nullptr) {
                translation = glm::mat3{1,0,0, 0,1,0, transform->uvOffset[0], transform->uvOffset[1], 1};
                rotation = glm::mat3{
                    cos(transform->rotation), sin(transform->rotation), 0,
                   -sin(transform->rotation), cos(transform->rotation), 0,
                                0,             0, 1
                };
                scale = glm::mat3{transform->uvScale[0],0,0, 0,transform->uvScale[1],0, 0,0,1};
            } else {
                translation = glm::mat3{1,0,0, 0,1,0, 0, 0, 1};
                rotation = glm::mat3{
                    cos(0.0), sin(0.0), 0,
                   -sin(0.0), cos(0.0), 0,
                                0,             0, 1
                };
                scale = glm::mat3{1,0,0, 0,1,0, 0,0,1};
            }
            return z0::ZRes::TextureInfo{
                .textureIndex = static_cast<int32_t>(sourceTextureInfo.textureIndex),
                .uvsIndex = static_cast<uint32_t>(sourceTextureInfo.texCoordIndex),
                .transform = translation * rotation * scale,
            };
        };
        if (material.pbrData.baseColorTexture.has_value()) {
            materialHeaders[materialIndex].albedoTexture = textureInfo(material.pbrData.baseColorTexture.value());
        }
        if (material.pbrData.metallicRoughnessTexture.has_value()) {
            materialHeaders[materialIndex].metallicTexture = textureInfo(material.pbrData.metallicRoughnessTexture.value());
            materialHeaders[materialIndex].roughnessTexture = textureInfo(material.pbrData.metallicRoughnessTexture.value());
        }
        if (material.normalTexture.has_value()) {
            materialHeaders[materialIndex].normalTexture = textureInfo(material.normalTexture.value());
            materialHeaders[materialIndex].normalScale = 1.0f;
        }
        if (material.emissiveTexture) {
            materialHeaders[materialIndex].emissiveTexture = textureInfo(material.emissiveTexture.value());
        }
        header.headersSize += sizeof(z0::ZRes::MaterialHeader);
    }

    // Fill meshes headers
    vector<uint32_t> indices{};
    vector<glm::vec3> positions{};
    vector<glm::vec3> normals{};
    vector<glm::vec2> uvs{};
    vector<glm::vec4> tangents{};
    auto meshesHeaders = vector<z0::ZRes::MeshHeader> {gltf.meshes.size()};
    auto surfaceInfo = vector<vector<z0::ZRes::SurfaceInfo>> {gltf.meshes.size()};
    auto uvsInfos = vector<vector<vector<z0::ZRes::DataInfo>>> {gltf.meshes.size()};
    for(auto meshIndex = 0; meshIndex < meshesHeaders.size(); meshIndex++) {
        auto& mesh = gltf.meshes[meshIndex];
        copyName(mesh.name.data(), meshesHeaders[meshIndex].name, meshIndex);
        meshesHeaders[meshIndex].surfacesCount = mesh.primitives.size();

        uvsInfos[meshIndex].resize(mesh.primitives.size());
        surfaceInfo[meshIndex].resize(mesh.primitives.size());

        uint32_t initialVtx{0};
        uint32_t uvsDataInfoCount{0};
        for(auto surfaceIndex = 0; surfaceIndex < mesh.primitives.size(); surfaceIndex++) {
            const auto& p = mesh.primitives[surfaceIndex];
            // load indexes
            {
                auto &indexAccessor = gltf.accessors[p.indicesAccessor.value()];
                surfaceInfo[meshIndex][surfaceIndex].indices.first = indices.size();
                surfaceInfo[meshIndex][surfaceIndex].indices.count = indexAccessor.count;
                indices.reserve(indices.size() + indexAccessor.count);
                fastgltf::iterateAccessor<std::uint32_t>(gltf, indexAccessor, [&](const uint32_t idx) {
                    indices.push_back(idx + initialVtx);
                });
            }
            // load vertex positions
            {
                auto &posAccessor = gltf.accessors[p.findAttribute("POSITION")->accessorIndex];
                surfaceInfo[meshIndex][surfaceIndex].positions.first = positions.size();
                surfaceInfo[meshIndex][surfaceIndex].positions.count = posAccessor.count;
                positions.reserve(positions.size() + posAccessor.count);
                fastgltf::iterateAccessor<glm::vec3>(gltf, posAccessor, [&](const glm::vec3 v) {
                    positions.push_back(v);
                    // cout << format("mesh {} surface {} position {}", meshIndex, surfaceIndex, to_string(v)) << endl;;
                });
                initialVtx += posAccessor.count;
            }
            // load vertex normals
            {
                auto normalAttr = p.findAttribute("NORMAL");
                if (normalAttr != p.attributes.end()) {
                    auto &normalAccessor = gltf.accessors[normalAttr->accessorIndex];
                    surfaceInfo[meshIndex][surfaceIndex].normals.first = normals.size();
                    surfaceInfo[meshIndex][surfaceIndex].normals.count = normalAccessor.count;
                    positions.reserve(normals.size() + normalAccessor.count);
                    fastgltf::iterateAccessor<glm::vec3>(gltf, normalAccessor, [&](const glm::vec3 v) {
                        normals.push_back(v);
                    });
                }
            }
            // load uv tangents
            {
                auto tangentAttr = p.findAttribute("TANGENT");
                if (tangentAttr != p.attributes.end()) {
                    auto &tangentAccessor = gltf.accessors[tangentAttr->accessorIndex];
                    surfaceInfo[meshIndex][surfaceIndex].tangents.first = tangents.size();
                    surfaceInfo[meshIndex][surfaceIndex].tangents.count = tangentAccessor.count;
                    tangents.reserve(tangents.size() + tangentAccessor.count);
                    fastgltf::iterateAccessorWithIndex<glm::vec4>(
                        gltf,tangentAccessor, [&](const glm::vec4 v, const size_t index) {
                            tangents.push_back(v);
                        });
                }
            }
            if (p.materialIndex.has_value()) {
                surfaceInfo[meshIndex][surfaceIndex].materialIndex = p.materialIndex.value();
                // load UVs
                surfaceInfo[meshIndex][surfaceIndex].uvsCount = 0;
                for(auto &attr : p.attributes) {
                    string name{attr.name.data()};
                    if (name.starts_with("TEXCOORD_")) {
                        auto& accessor = gltf.accessors[attr.accessorIndex];
                        uvsDataInfoCount += 1;
                        uvsInfos[meshIndex][surfaceIndex].resize(surfaceInfo[meshIndex][surfaceIndex].uvsCount + 1);
                        uvsInfos[meshIndex][surfaceIndex][surfaceInfo[meshIndex][surfaceIndex].uvsCount].first = uvs.size();
                        uvsInfos[meshIndex][surfaceIndex][surfaceInfo[meshIndex][surfaceIndex].uvsCount].count = accessor.count;
                        uvs.reserve(uvs.size() + accessor.count);
                        fastgltf::iterateAccessor<glm::vec2>(gltf, accessor, [&](const glm::vec2 v) {
                            uvs.push_back({v.x, v.y});
                        });
                        surfaceInfo[meshIndex][surfaceIndex].uvsCount += 1;
                    }
                }
            } else {
                surfaceInfo[meshIndex][surfaceIndex].materialIndex = -1;
            }
        }
        header.headersSize += sizeof(z0::ZRes::MeshHeader)
                            + sizeof(z0::ZRes::SurfaceInfo) * surfaceInfo[meshIndex].size()
                            + sizeof(z0::ZRes::DataInfo) * uvsDataInfoCount;
    }

    // Fill the nodes header
    auto nodesHeaders = vector<z0::ZRes::NodeHeader> { gltf.nodes.size()};
    auto childrenIndexes = vector<vector<uint32_t>>  {gltf.nodes.size()};
    for (auto nodeIndex = 0; nodeIndex < gltf.nodes.size(); ++nodeIndex) {
        const auto &node = gltf.nodes[nodeIndex];
        copyName(node.name.data(), nodesHeaders[nodeIndex].name, nodeIndex);
        nodesHeaders[nodeIndex].meshIndex = node.meshIndex.has_value() ? node.meshIndex.value() : -1;
        visit(fastgltf::visitor{
              [&](fastgltf::math::fmat4x4 matrix) {
                  memcpy(&nodesHeaders[nodeIndex].transform, matrix.data(), sizeof(glm::mat4));
              },
              [&](fastgltf::TRS transform) {
                  const glm::vec3 tl(transform.translation[0], transform.translation[1], transform.translation[2]);
                  const glm::quat rot(transform.rotation[3],
                           transform.rotation[0],
                           transform.rotation[1],
                           transform.rotation[2]);
                  const glm::vec3 sc(transform.scale[0], transform.scale[1], transform.scale[2]);
                  const glm::mat4 trm = glm::translate(glm::mat4(1.f), tl);
                  const glm::mat4 rm = glm::toMat4(rot);
                  const glm::mat4 sm = glm::scale(glm::mat4(1.f), sc);
                  nodesHeaders[nodeIndex].transform = trm * rm * sm;
              }},
            node.transform);
        childrenIndexes[nodeIndex].resize(node.children.size());
        for(auto i = 0; i < node.children.size(); ++i) {
            childrenIndexes[nodeIndex][i] = node.children[i];
        }
        nodesHeaders[nodeIndex].childrenCount = node.children.size();
        header.headersSize += sizeof(z0::ZRes::NodeHeader) +
                            sizeof(uint32_t) * childrenIndexes[nodeIndex].size();
    }
    if (verbose) { z0::ZRes::print(header); }

    // header.headersPadding = calculatePadding(header.headersSize, VK_FORMAT_BC1_RGBA_UNORM_BLOCK); // use largest padding
    // header.meshesDataPadding = calculatePadding(
            // indices.size() * sizeof(uint32_t) +
            // positions.size() * sizeof(glm::vec3) +
            // normals.size() * sizeof(glm::vec3) +
            // uvs.size() * sizeof(glm::vec2) +
            // tangents.size() * sizeof(glm::vec4),
        // VK_FORMAT_BC1_RGBA_UNORM_BLOCK);

    if (verbose) { cout << "Writing output file " << outputFilename << "...\n"; }
    std::ofstream outputFile(outputFilename, std::ios::binary);
    if (!outputFile) {
        cerr << "Error opening file " << outputFilename.string() << "for writing!\n";
        return EXIT_FAILURE;
    }

    // Write all the headers
    if (verbose) { cout << "\tHeaders...\n"; }
    outputFile.write(reinterpret_cast<const char*>(&header), sizeof(z0::ZRes::Header));
    for (auto imageIndex = 0; imageIndex < gltf.images.size(); imageIndex++) {
        // z0::ZScene::print(imageHeaders[imageIndex]);
        outputFile.write(reinterpret_cast<const char*>(&imageHeaders[imageIndex]),sizeof(z0::ZRes::ImageHeader));
        outputFile.write(reinterpret_cast<const ostream::char_type *>(mipHeaders[imageIndex].data()), mipHeaders[imageIndex].size() * sizeof(z0::ZRes::MipLevelInfo));
    }
    outputFile.write(reinterpret_cast<const char*>(textureHeaders.data()),textureHeaders.size() * sizeof(z0::ZRes::TextureHeader));
    outputFile.write(reinterpret_cast<const char*>(materialHeaders.data()),materialHeaders.size() * sizeof(z0::ZRes::MaterialHeader));
    for (auto meshIndex = 0; meshIndex < gltf.meshes.size(); meshIndex++) {
        // z0::ZScene::print(meshesHeaders[meshIndex]);
        outputFile.write(reinterpret_cast<const char*>(&meshesHeaders[meshIndex]),sizeof(z0::ZRes::MeshHeader));
        for(auto surfaceIndex = 0; surfaceIndex < meshesHeaders[meshIndex].surfacesCount; surfaceIndex++) {
            // ZScene::print(surfaceInfo[meshIndex][surfaceIndex]);
            outputFile.write(reinterpret_cast<const char*>(&surfaceInfo[meshIndex][surfaceIndex]),sizeof(z0::ZRes::SurfaceInfo));
            // ZScene::print(uvsInfos[meshIndex][surfaceIndex][0]);
            // ZScene::print(uvsInfos[meshIndex][surfaceIndex][1]);
            outputFile.write(reinterpret_cast<const char*>(uvsInfos[meshIndex][surfaceIndex].data()), uvsInfos[meshIndex][surfaceIndex].size() * sizeof(z0::ZRes::DataInfo));
        }
    }
    for (auto nodeIndex = 0; nodeIndex < gltf.nodes.size(); ++nodeIndex) {
        outputFile.write(reinterpret_cast<const char*>(&nodesHeaders[nodeIndex]),sizeof(z0::ZRes::NodeHeader));
        outputFile.write(reinterpret_cast<const char*>(childrenIndexes[nodeIndex].data()),childrenIndexes[nodeIndex].size() * sizeof(uint32_t));
    }

    // auto pad = vector<uint8_t>(vector<uint8_t>::size_type(32), 0);
    // if (header.headersPadding > 0) {
        // if (verbose) { cout << "Adding header padding for alignment: " << header.headersPadding << endl; };
        // outputFile.write(reinterpret_cast<const char*>(pad.data()), header.headersPadding);
    // }

    // Write meshes data
    if (verbose) { cout << "\tMeshes data...\n"; }
    uint32_t count = indices.size();
    outputFile.write(reinterpret_cast<const char*>(&count),sizeof(uint32_t));
    outputFile.write(reinterpret_cast<const char*>(indices.data()),count * sizeof(uint32_t));

    count = positions.size();
    outputFile.write(reinterpret_cast<const char*>(&count),sizeof(uint32_t));
    outputFile.write(reinterpret_cast<const char*>(positions.data()),count * sizeof(glm::vec3));

    count = normals.size();
    outputFile.write(reinterpret_cast<const char*>(&count),sizeof(uint32_t));
    outputFile.write(reinterpret_cast<const char*>(normals.data()),count * sizeof(glm::vec3));

    count = uvs.size();
    outputFile.write(reinterpret_cast<const char*>(&count),sizeof(uint32_t));
    outputFile.write(reinterpret_cast<const char*>(uvs.data()),count * sizeof(glm::vec2));

    count = tangents.size();
    outputFile.write(reinterpret_cast<const char*>(&count),sizeof(uint32_t));
    outputFile.write(reinterpret_cast<const char*>(tangents.data()),count * sizeof(glm::vec4));

    // if (header.meshesDataPadding > 0) {
        // if (verbose) { cout << "Adding meshes data padding for alignment: " << header.meshesDataPadding << endl; };
        // outputFile.write(reinterpret_cast<const char*>(pad.data()), header.meshesDataPadding);
    // }

    // cout << format("{} indices, {} positions, {} normals, {} uvs, {} tangents",
        // indices.size(), positions.size(), normals.size(), uvs.size(), tangents.size()) << endl;
    // for(const auto&p : positions) {
        // cout << to_string(p) << endl;
    // }

    // Write images
    if (verbose) { cout << "\tImages data...\n"; }
    for (auto imageIndex = 0; imageIndex < gltf.images.size(); imageIndex++) {
        const auto& mipOut = outMipLevels.at(imageIndex);
        // saveImageDDS("out" + to_string(imageIndex) + ".dds", mipOut, formatName);
        // z0::ZScene::print(imageHeaders[imageIndex]);
        // if (verbose) {
            // std::printf("Writing image %d %dx%d...\n", imageIndex,
                // mipOut[0].width,
                // mipOut[0].height);
        // }
        for(auto mipLevel = 0; mipLevel <mipOut.size(); ++mipLevel) {
            // std::printf("Writing image %d level %d %dx%d (%llu, %llu, %llu)...\n", imageIndex, mipLevel,
            //     mipOut[mipLevel].width,
            //     mipOut[mipLevel].height,
            //     mipOut[mipLevel].dataSize,
            //     mipHeaders[imageIndex][mipLevel].offset,
            //     mipHeaders[imageIndex][mipLevel].size);
            outputFile.write(
                reinterpret_cast<const char*>(mipOut[mipLevel].data->data()),
                mipOut[mipLevel].data->size());
            if (!outputFile) {
                cerr << "Error writing to file!" << endl;
                return 1;
            }
            // if (mipHeaders[imageIndex][mipLevel].padding > 0) {
                 // if (verbose) { cout << "Adding level padding: " << mipHeaders[imageIndex][mipLevel].padding << endl;}
                 // outputFile.write(reinterpret_cast<const char*>(pad.data()), mipHeaders[imageIndex][mipLevel].padding);
            // }
        }
        // if (imageHeaders[imageIndex].padding > 0) {
             // if (verbose) {cout << "Adding image padding: " << imageHeaders[imageIndex].padding << endl; }
             // outputFile.write(reinterpret_cast<const char*>(pad.data()), imageHeaders[imageIndex].padding);
        // }
    }

    outputFile.close();
    return 0;
}
