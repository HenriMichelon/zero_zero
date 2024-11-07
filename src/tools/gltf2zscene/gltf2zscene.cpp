#define GLM_ENABLE_EXPERIMENTAL
import glm;
using namespace glm;

#include <compressonator.h>
#include <cxxopts.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <stb_image.h>
#include <volk.h>
#include <glm/gtx/quaternion.hpp>

import std;
using namespace std;
namespace fs = std::filesystem;

import z0;
using namespace z0;

// https://compressonator.readthedocs.io/en/latest/developer_sdk/cmp_framework/index.html#using-the-pipeline-api-interfaces
void convertImage(const string& name,
    const unsigned char* data,
    const int width, const int height,
    CMP_MipSet& MipSetIn, CMP_MipSet& MipSetCmp,
    const CMP_FORMAT format) {
    memset(&MipSetIn, 0, sizeof(CMP_MipSet));
    if (CMP_CreateMipSet(&MipSetIn, width, height, 1, CF_8bit, TT_2D) != CMP_OK) {
        die("Error while creating Mip Set ", name);
    }
    memcpy(MipSetIn.pData, data, MipSetIn.dwDataSize);
    MipSetIn.m_format     = CMP_FORMAT_RGBA_8888;
    MipSetIn.m_nBlockWidth = 0;
    MipSetIn.m_nBlockHeight = 0;

    //----------------------------------------------------------------------
    // generate mipmap level for the source image, if not already generated
    //----------------------------------------------------------------------
    if (MipSetIn.m_nMipLevels <= 1) {
        // cout << "Generating mimaps for " << name << "...\n";
        const CMP_INT requestLevel = static_cast<int>(std::floor(std::log2(std::max(MipSetIn.m_nWidth, MipSetIn.m_nHeight))));
        const CMP_INT nMinSize = CMP_CalcMinMipSize(MipSetIn.m_nHeight, MipSetIn.m_nWidth, requestLevel);
        CMP_GenerateMIPLevels(&MipSetIn, nMinSize);
    }

    //==========================
    // Set Compression Options
    //==========================
    const KernelOptions   kernel_options{
        .fquality = 1.0,
        .format   = format,
        .encodeWith = CMP_HPC ,
        .threads  = 0,
    };

    //===============================================
    // Compress the texture using Compressonator Lib
    //===============================================
    // cout << "Compressing image " << name << "...\n";
    memset(&MipSetCmp, 0, sizeof(CMP_MipSet));
    auto cmp_status = CMP_ProcessTexture(&MipSetIn, &MipSetCmp, kernel_options, nullptr);
    if (cmp_status != CMP_OK) {
        CMP_FreeMipSet(&MipSetIn);
        die("Compression returned an error ", to_string(cmp_status));
    }
}

auto convertThread = vector<thread>();
void loadAndConvertImage(const string&  name,
                               const void   * srcData,
                               const size_t   size,
                               CMP_MipSet& mipSetIn,
                               CMP_MipSet& mipSetCmp) {
    convertThread.push_back(thread([&mipSetIn, &mipSetCmp, name, srcData, size]() {
        int width, height, channels;
        auto *data = stbi_load_from_memory(static_cast<stbi_uc const *>(srcData),
                                                     static_cast<int>(size),
                                                     &width,
                                                     &height,
                                                     &channels,
                                                     STBI_rgb_alpha);
        if (data) {
             convertImage(name, data, width, height, mipSetIn, mipSetCmp, CMP_FORMAT_BC1);
             stbi_image_free(data);
             cout << "Converted image " << name << endl;
        }
    }));
};

void loadImage(
     fastgltf::Asset &asset,
     fastgltf::Image &image,
     CMP_MipSet& mipSetIn,
     CMP_MipSet& mipSetCmp) {
    shared_ptr<Image> newImage;
    const auto& name = image.name.data();
    visit(fastgltf::visitor{
      [](auto &arg) {},
      [&](fastgltf::sources::URI &filePath) {
          die("External textures files for glTF not supported");
      },
      [&](fastgltf::sources::Vector &vector) {
          loadAndConvertImage(name, vector.bytes.data(), vector.bytes.size(), mipSetIn, mipSetCmp);
      },
      [&](fastgltf::sources::BufferView &view) {
          const auto &bufferView = asset.bufferViews[view.bufferViewIndex];
          auto &buffer           = asset.buffers[bufferView.bufferIndex];
          visit(fastgltf::visitor{
                [](auto &arg) {},
                [&](fastgltf::sources::Vector &vector) {
                    loadAndConvertImage(name, vector.bytes.data() + bufferView.byteOffset, bufferView.byteLength, mipSetIn, mipSetCmp);
                },
                [&](fastgltf::sources::Array &array) {
                    loadAndConvertImage(name, array.bytes.data() + bufferView.byteOffset, bufferView.byteLength, mipSetIn, mipSetCmp);
                },
            },
            buffer.data);
      },
      },
    image.data);
}

int main(const int argc, char** argv) {
    cxxopts::Options options("gltl2zscene", "Create a ZScene file from a glTF binary file");
    options.add_options()
        ("input", "The binary glTF file to read", cxxopts::value<string>())
        ("output","The ZScene file to create", cxxopts::value<string>());
    options.parse_positional({"input", "output"});
    const auto result = options.parse(argc, argv);
    if (result.count("input") != 1 || result.count("output") != 1) {
        std::cerr << "usage: gltl2zscene input.glb output.zscene\n";
        return EXIT_FAILURE;
    }
    const auto &inputFilename  = fs::path(result["input"].as<string>());
    const auto &outputFilename = fs::path(result["output"].as<string>());

    // Open the glTF file
    constexpr auto gltfOptions =
        fastgltf::Options::DontRequireValidAssetMember |
        fastgltf::Options::AllowDouble |
        fastgltf::Options::LoadGLBBuffers |
        fastgltf::Options::LoadExternalBuffers;
    auto gltfFile = fastgltf::GltfDataBuffer::FromPath(inputFilename);
    if (auto error = gltfFile.error(); error != fastgltf::Error::None) {
        die(getErrorMessage(error));
    }

    // Read the glTF JSON file or GLB headers
    fastgltf::Parser parser{fastgltf::Extensions::KHR_materials_specular |
                            fastgltf::Extensions::KHR_texture_transform |
                            fastgltf::Extensions::KHR_materials_emissive_strength};
    auto asset = parser.loadGltf(gltfFile.get(), inputFilename.parent_path(), gltfOptions);
    if (auto error = asset.error(); error != fastgltf::Error::None) {
        die(getErrorMessage(error));
    }
    fastgltf::Asset gltf = std::move(asset.get());

    // Load & transcode all the images
    CMP_InitFramework();
    CMP_InitializeBCLibrary();
    auto MipSetIn = vector<CMP_MipSet>(gltf.images.size());
    auto MipSetCmp = vector<CMP_MipSet>(gltf.images.size());
    auto tStart = std::chrono::high_resolution_clock::now();
    for (auto imageIndex = 0; imageIndex < gltf.images.size(); imageIndex++) {
        loadImage(gltf, gltf.images[imageIndex], MipSetIn[imageIndex],MipSetCmp[imageIndex]);
    }
    for(auto& t : convertThread) {
        t.join();
    }
    CMP_ShutdownBCLibrary();
    auto last_transcode_time = std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - tStart).count();
    cout << "total transcoding time for "  << gltf.images.size() << " images : " <<  last_transcode_time << "ms" << endl;

    // Initialize the destination file headers
    auto header = ZScene::Header {
        .version = 1,
        .imagesCount = static_cast<uint32_t>(gltf.images.size()),
        .texturesCount = static_cast<uint32_t>(gltf.textures.size()),
        .materialsCount = static_cast<uint32_t>(gltf.materials.size()),
        .meshesCount = static_cast<uint32_t>(gltf.meshes.size()),
        .nodesCount = static_cast<uint32_t>(gltf.nodes.size()),
        .headersSize = sizeof(ZScene::Header),
    };
    header.magic[0] = ZScene::MAGIC[0];
    header.magic[1] = ZScene::MAGIC[1];
    header.magic[2] = ZScene::MAGIC[2];
    header.magic[3] = ZScene::MAGIC[3];

    // Fill the images headers
    auto imageHeaders = vector<ZScene::ImageHeader> {gltf.images.size()};
    auto mipHeaders = vector<vector<ZScene::MipLevelInfo>>{gltf.images.size()};
    uint64_t dataOffset = 0;
    for(auto imageIndex = 0; imageIndex < gltf.images.size(); imageIndex++) {
        string name = gltf.images[imageIndex].name.data();
        if (name.empty()) { name = to_string(imageIndex); }
        strncpy(imageHeaders[imageIndex].name, name.c_str(), ZScene::NAME_SIZE);
        imageHeaders[imageIndex].name[ZScene::NAME_SIZE - 1] = '\0';

        // Set header for each mip level of the image
        uint64_t mipOffset{0};
        mipHeaders[imageIndex].resize(MipSetIn[imageIndex].m_nMipLevels);
        for(auto mipLevel = 0; mipLevel < MipSetCmp[imageIndex].m_nMipLevels; ++mipLevel) {
            mipHeaders[imageIndex][mipLevel].offset = mipOffset;
            mipHeaders[imageIndex][mipLevel].size = MipSetCmp[imageIndex].m_pMipLevelTable[mipLevel]->m_dwLinearSize;
            mipOffset += mipHeaders[imageIndex][mipLevel].size;
        }

        // Set header for one image
        imageHeaders[imageIndex].format = VK_FORMAT_BC1_RGB_SRGB_BLOCK;
        imageHeaders[imageIndex].width  = static_cast<uint32_t>(MipSetIn[imageIndex].m_nWidth);
        imageHeaders[imageIndex].height  = static_cast<uint32_t>(MipSetIn[imageIndex].m_nHeight);
        imageHeaders[imageIndex].mipLevels  = static_cast<uint32_t>(MipSetIn[imageIndex].m_nMipLevels);
        imageHeaders[imageIndex].dataOffset = dataOffset;
        imageHeaders[imageIndex].dataSize = mipOffset;
        dataOffset += mipOffset;

        header.headersSize += sizeof(ZScene::ImageHeader) + sizeof(ZScene::MipLevelInfo) * MipSetIn[imageIndex].m_nMipLevels;
    }

    // Fill textures headers
    auto textureHeaders = vector<ZScene::TextureHeader> {gltf.textures.size()};
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
        header.headersSize += sizeof(ZScene::TextureHeader);
    }

    // Fill materials headers
    auto materialHeaders = vector<ZScene::MaterialHeader> {gltf.materials.size()};
    for(auto materialIndex = 0; materialIndex < gltf.materials.size(); materialIndex++) {
        const auto& material = gltf.materials[materialIndex];

        string name =material.name.data();
        if (name.empty()) { name = to_string(materialIndex); }
        strncpy(materialHeaders[materialIndex].name, name.c_str(), ZScene::NAME_SIZE);
        materialHeaders[materialIndex].name[ZScene::NAME_SIZE - 1] = '\0';

        materialHeaders[materialIndex].albedoColor = {
            material.pbrData.baseColorFactor[0],
            material.pbrData.baseColorFactor[1],
            material.pbrData.baseColorFactor[2],
            material.pbrData.baseColorFactor[3],
        };
        materialHeaders[materialIndex].metallicFactor = material.pbrData.metallicFactor;
        materialHeaders[materialIndex].roughnessFactor = material.pbrData.roughnessFactor;
        materialHeaders[materialIndex].emissiveFactor = vec3{material.emissiveFactor[0], material.emissiveFactor[1], material.emissiveFactor[2]};
        materialHeaders[materialIndex].emissiveStrength = material.emissiveStrength;
        switch (material.alphaMode) {
        case fastgltf::AlphaMode::Blend:
            materialHeaders[materialIndex].transparency = static_cast<uint32_t>(Transparency::ALPHA);
            break;
        case fastgltf::AlphaMode::Mask:
            materialHeaders[materialIndex].transparency =  static_cast<uint32_t>(Transparency::SCISSOR);
            materialHeaders[materialIndex].alphaScissor = material.alphaCutoff;
            break;
        default:
            break;
        }
        materialHeaders[materialIndex].cullMode =  static_cast<uint32_t>(material.doubleSided ? CullMode::DISABLED : CullMode::BACK);
        auto textureInfo = [](const fastgltf::TextureInfo& sourceTextureInfo) {
            mat3 translation;
            mat3 scale;
            mat3 rotation;
            const auto& transform = sourceTextureInfo.transform;
            if (transform != nullptr) {
                translation = mat3{1,0,0, 0,1,0, transform->uvOffset[0], transform->uvOffset[1], 1};
                rotation = mat3{
                    cos(transform->rotation), sin(transform->rotation), 0,
                   -sin(transform->rotation), cos(transform->rotation), 0,
                                0,             0, 1
                };
                scale = mat3{transform->uvScale[0],0,0, 0,transform->uvScale[1],0, 0,0,1};
            } else {
                translation = mat3{1,0,0, 0,1,0, 0, 0, 1};
                rotation = mat3{
                    cos(0.0), sin(0.0), 0,
                   -sin(0.0), cos(0.0), 0,
                                0,             0, 1
                };
                scale = mat3{1,0,0, 0,1,0, 0,0,1};
            }
            return ZScene::TextureInfo{
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
            imageHeaders[materialHeaders[materialIndex].metallicTexture.textureIndex].format = VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        }
        if (material.normalTexture.has_value()) {
            materialHeaders[materialIndex].normalTexture = textureInfo(material.normalTexture.value());
            materialHeaders[materialIndex].normalScale = 1.0f;
            imageHeaders[materialHeaders[materialIndex].normalTexture.textureIndex].format = VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        }
        if (material.emissiveTexture) {
            materialHeaders[materialIndex].emissiveTexture = textureInfo(material.emissiveTexture.value());
        }
        header.headersSize += sizeof(ZScene::MaterialHeader);
    }

    // Fill meshes headers
    vector<uint32_t> indices{};
    vector<vec3> positions{};
    vector<vec3> normals{};
    vector<vec2> uvs{};
    vector<vec4> tangents{};
    auto meshesHeaders = vector<ZScene::MeshHeader> {gltf.meshes.size()};
    auto surfaceInfo = vector<vector<ZScene::SurfaceInfo>> {gltf.meshes.size()};
    auto uvsInfos = vector<vector<vector<ZScene::DataInfo>>> {gltf.meshes.size()};
    for(auto meshIndex = 0; meshIndex < meshesHeaders.size(); meshIndex++) {
        auto& mesh = gltf.meshes[meshIndex];

        string name = mesh.name.data();
        if (name.empty()) { name = to_string(meshIndex); }
        strncpy(meshesHeaders[meshIndex].name, name.c_str(), ZScene::NAME_SIZE);
        meshesHeaders[meshIndex].name[ZScene::NAME_SIZE - 1] = '\0';
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
                fastgltf::iterateAccessor<vec3>(gltf, posAccessor, [&](const vec3 v) {
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
                    fastgltf::iterateAccessor<vec3>(gltf, normalAccessor, [&](const vec3 v) {
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
                    fastgltf::iterateAccessorWithIndex<vec4>(
                        gltf,tangentAccessor, [&](const vec4 v, const size_t index) {
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
                        fastgltf::iterateAccessor<vec2>(gltf, accessor, [&](const vec2 v) {
                            uvs.push_back({v.x, v.y});
                        });
                        surfaceInfo[meshIndex][surfaceIndex].uvsCount += 1;
                    }
                }
            } else {
                surfaceInfo[meshIndex][surfaceIndex].materialIndex = -1;
            }
        }
        header.headersSize += sizeof(ZScene::MeshHeader)
                            + sizeof(ZScene::SurfaceInfo) * surfaceInfo[meshIndex].size()
                            + sizeof(ZScene::DataInfo) * uvsDataInfoCount;
    }

    // Fill the nodes header
    auto nodesHeaders = vector<ZScene::NodeHeader> { gltf.nodes.size()};
    auto childrenIndexes = vector<vector<uint32_t>>  {gltf.nodes.size()};
    for (auto nodeIndex = 0; nodeIndex < gltf.nodes.size(); ++nodeIndex) {
        const auto &node = gltf.nodes[nodeIndex];

        string name = node.name.data();
        if (name.empty()) { name = to_string(nodeIndex); }
        strncpy(nodesHeaders[nodeIndex].name, name.c_str(), ZScene::NAME_SIZE);
        nodesHeaders[nodeIndex].name[ZScene::NAME_SIZE - 1] = '\0';
        nodesHeaders[nodeIndex].meshIndex = node.meshIndex.has_value() ? node.meshIndex.value() : -1;

        visit(fastgltf::visitor{
              [&](fastgltf::math::fmat4x4 matrix) {
                  memcpy(&nodesHeaders[nodeIndex].transform, matrix.data(), sizeof(mat4));
              },
              [&](fastgltf::TRS transform) {
                  const vec3 tl(transform.translation[0], transform.translation[1], transform.translation[2]);
                  const quat rot(transform.rotation[3],
                           transform.rotation[0],
                           transform.rotation[1],
                           transform.rotation[2]);
                  const vec3 sc(transform.scale[0], transform.scale[1], transform.scale[2]);
                  const mat4 trm = translate(mat4(1.f), tl);
                  const mat4 rm = toMat4(rot);
                  const mat4 sm = scale(mat4(1.f), sc);
                  nodesHeaders[nodeIndex].transform = trm * rm * sm;
              }},
            node.transform);
        childrenIndexes[nodeIndex].resize(node.children.size());
        for(auto i = 0; i < node.children.size(); ++i) {
            childrenIndexes[nodeIndex][i] = node.children[i];
        }
        nodesHeaders[nodeIndex].childrenCount = node.children.size();
        header.headersSize += sizeof(ZScene::NodeHeader) +
                            sizeof(uint32_t) * childrenIndexes[nodeIndex].size();
    }

    cout << "Writing output file " << outputFilename << "...\n";
    std::ofstream outputFile(outputFilename, std::ios::binary);
    if (!outputFile) {
        die("Error opening file for writing!");
        return 1;
    }

    // Write all the headers
    // ZScene::print(header);
    outputFile.write(reinterpret_cast<const char*>(&header), sizeof(ZScene::Header));
    for (auto imageIndex = 0; imageIndex < gltf.images.size(); imageIndex++) {
        // ZScene::print(imageHeaders[imageIndex]);
        outputFile.write(reinterpret_cast<const char*>(&imageHeaders[imageIndex]),sizeof(ZScene::ImageHeader));
        outputFile.write(reinterpret_cast<const ostream::char_type *>(mipHeaders[imageIndex].data()), mipHeaders[imageIndex].size() * sizeof(ZScene::MipLevelInfo));
    }
    outputFile.write(reinterpret_cast<const char*>(textureHeaders.data()),textureHeaders.size() * sizeof(ZScene::TextureHeader));
    outputFile.write(reinterpret_cast<const char*>(materialHeaders.data()),materialHeaders.size() * sizeof(ZScene::MaterialHeader));
    for (auto meshIndex = 0; meshIndex < gltf.meshes.size(); meshIndex++) {
        // ZScene::print(meshesHeaders[meshIndex]);
        outputFile.write(reinterpret_cast<const char*>(&meshesHeaders[meshIndex]),sizeof(ZScene::MeshHeader));
        for(auto surfaceIndex = 0; surfaceIndex < meshesHeaders[meshIndex].surfacesCount; surfaceIndex++) {
            // ZScene::print(surfaceInfo[meshIndex][surfaceIndex]);
            outputFile.write(reinterpret_cast<const char*>(&surfaceInfo[meshIndex][surfaceIndex]),sizeof(ZScene::SurfaceInfo));
            // ZScene::print(uvsInfos[meshIndex][surfaceIndex][0]);
            // ZScene::print(uvsInfos[meshIndex][surfaceIndex][1]);
            outputFile.write(reinterpret_cast<const char*>(uvsInfos[meshIndex][surfaceIndex].data()), uvsInfos[meshIndex][surfaceIndex].size() * sizeof(ZScene::DataInfo));
        }
    }
    for (auto nodeIndex = 0; nodeIndex < gltf.nodes.size(); ++nodeIndex) {
        outputFile.write(reinterpret_cast<const char*>(&nodesHeaders[nodeIndex]),sizeof(ZScene::NodeHeader));
        outputFile.write(reinterpret_cast<const char*>(childrenIndexes[nodeIndex].data()),childrenIndexes[nodeIndex].size() * sizeof(uint32_t));
    }

    // Write meshes data
    uint32_t count = indices.size();
    outputFile.write(reinterpret_cast<const char*>(&count),sizeof(uint32_t));
    outputFile.write(reinterpret_cast<const char*>(indices.data()),indices.size() * sizeof(uint32_t));

    count = positions.size();
    outputFile.write(reinterpret_cast<const char*>(&count),sizeof(uint32_t));
    outputFile.write(reinterpret_cast<const char*>(positions.data()),positions.size() * sizeof(vec3));

    count = normals.size();
    outputFile.write(reinterpret_cast<const char*>(&count),sizeof(uint32_t));
    outputFile.write(reinterpret_cast<const char*>(normals.data()),normals.size() * sizeof(vec3));

    count = uvs.size();
    outputFile.write(reinterpret_cast<const char*>(&count),sizeof(uint32_t));
    outputFile.write(reinterpret_cast<const char*>(uvs.data()),uvs.size() * sizeof(vec2));

    count = tangents.size();
    outputFile.write(reinterpret_cast<const char*>(&count),sizeof(uint32_t));
    outputFile.write(reinterpret_cast<const char*>(tangents.data()),tangents.size() * sizeof(vec4));

    // cout << format("{} indices, {} positions, {} normals, {} uvs, {} tangents",
        // indices.size(), positions.size(), normals.size(), uvs.size(), tangents.size()) << endl;
    // for(const auto&p : positions) {
        // cout << to_string(p) << endl;
    // }

    // Write images
    for (auto imageIndex = 0; imageIndex < gltf.images.size(); imageIndex++) {
        for(auto mipLevel = 0; mipLevel < MipSetCmp[imageIndex].m_nMipLevels; ++mipLevel) {
            // ZScene::print(mipHeaders[imageIndex][mipLevel]);
            // std::printf("Writing image %d level %d %dx%d (%d)...\n", imageIndex, mipLevel,
            //     MipSetCmp[imageIndex].m_pMipLevelTable[mipLevel]->m_nWidth,
            //     MipSetCmp[imageIndex].m_pMipLevelTable[mipLevel]->m_nHeight,
            //     MipSetCmp[imageIndex].m_pMipLevelTable[mipLevel]->m_dwLinearSize);
            outputFile.write(reinterpret_cast<const char*>(MipSetCmp[imageIndex].m_pMipLevelTable[mipLevel]->m_pbData),MipSetCmp[imageIndex].m_pMipLevelTable[mipLevel]->m_dwLinearSize);
            if (!outputFile) {
                die("Error writing to file!");
                return 1;
            }
        }
    }
    for (auto imageIndex = 0; imageIndex < gltf.images.size(); imageIndex++) {
        CMP_FreeMipSet(&MipSetIn[imageIndex]);
        CMP_FreeMipSet(&MipSetCmp[imageIndex]);
    }

    outputFile.close();
    return 0;
}
