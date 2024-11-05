#include <compressonator.h>
#include <cxxopts.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <stb_image.h>
#include <volk.h>

import std;
using namespace std;
namespace fs = std::filesystem;

import z0;
using namespace z0;

// https://compressonator.readthedocs.io/en/latest/developer_sdk/cmp_framework/index.html#using-the-pipeline-api-interfaces
void convert(const string& name, unsigned char* data, int width, int height, CMP_MipSet& MipSetIn, CMP_MipSet& MipSetCmp, const CMP_FORMAT format) {
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


typedef std::function<void(
            const string&   name,
            const void   *  srcData,
            size_t          size,
            CMP_MipSet& mipSetIn,
            CMP_MipSet& mipSetCmp)> LoadImageFunction;


void loadImage(
     fastgltf::Asset &asset,
     fastgltf::Image &image,
     CMP_MipSet& mipSetIn,
     CMP_MipSet& mipSetCmp,
     const LoadImageFunction& loadImageFunction) {
    shared_ptr<Image> newImage;
    const auto& name = image.name.data();
    visit(fastgltf::visitor{
      [](auto &arg) {die("1");},
      [&](fastgltf::sources::URI &filePath) {
          die("External textures files for glTF not supported");
      },
      [&](fastgltf::sources::Vector &vector) {
          loadImageFunction(name, vector.bytes.data(), vector.bytes.size(), mipSetIn, mipSetCmp);
      },
      [&](fastgltf::sources::BufferView &view) {
          const auto &bufferView = asset.bufferViews[view.bufferViewIndex];
          auto &buffer           = asset.buffers[bufferView.bufferIndex];
          visit(fastgltf::visitor{
                [](auto &arg) { die("2"); },
                [&](fastgltf::sources::Vector &vector) {
                    loadImageFunction(name, vector.bytes.data() + bufferView.byteOffset, bufferView.byteLength, mipSetIn, mipSetCmp);
                },
                [&](fastgltf::sources::Array &array) {
                    loadImageFunction(name, array.bytes.data() + bufferView.byteOffset, bufferView.byteLength, mipSetIn, mipSetCmp);
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

    CMP_InitFramework();
    CMP_InitializeBCLibrary();

    fastgltf::Parser parser{fastgltf::Extensions::KHR_materials_specular |
                            fastgltf::Extensions::KHR_texture_transform |
                            fastgltf::Extensions::KHR_materials_emissive_strength};
    constexpr auto gltfOptions =
        fastgltf::Options::DontRequireValidAssetMember |
        fastgltf::Options::AllowDouble |
        fastgltf::Options::LoadGLBBuffers |
        fastgltf::Options::LoadExternalBuffers;
    auto gltfFile = fastgltf::GltfDataBuffer::FromPath(inputFilename);
    if (auto error = gltfFile.error(); error != fastgltf::Error::None) {
        die(getErrorMessage(error));
    }
    auto asset = parser.loadGltf(gltfFile.get(), inputFilename.parent_path(), gltfOptions);
    if (auto error = asset.error(); error != fastgltf::Error::None) {
        die(getErrorMessage(error));
    }
    fastgltf::Asset gltf = std::move(asset.get());

    auto convertThread = vector<thread>();
    auto convertImageRGBA = [&](const string&  name,
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
                 convert(name, data, width, height, mipSetIn, mipSetCmp, CMP_FORMAT_BC1);
                 stbi_image_free(data);
                 cout << "Converted image " << name << endl;
            }
        }));
    };

    auto MipSetIn = vector<CMP_MipSet>(gltf.images.size());
    auto MipSetCmp = vector<CMP_MipSet>(gltf.images.size());
    int imageIndex = 0;
    auto loadTexture = [&](const fastgltf::TextureInfo& sourceTextureInfo) {
        const auto& texture = gltf.textures[sourceTextureInfo.textureIndex];
        if (texture.imageIndex.has_value()) {
            loadImage(gltf, gltf.images[texture.imageIndex.value()],
                MipSetIn[imageIndex],MipSetCmp[imageIndex],
                convertImageRGBA);
            imageIndex += 1;
        }
    };
        auto tStart = std::chrono::high_resolution_clock::now();
    for (auto &mat : gltf.materials) {
        cout << "Extracting material " << mat.name << endl;
        if (mat.pbrData.baseColorTexture.has_value()) {
            loadTexture(mat.pbrData.baseColorTexture.value());
        }
        // if (mat.pbrData.metallicRoughnessTexture.has_value()) {
        //     loadTexture(mat.pbrData.metallicRoughnessTexture.value());
        // }
        // if (mat.normalTexture.has_value()) {
        //     loadTexture(mat.normalTexture.value());
        // }
        // if (mat.emissiveTexture) {
        //     loadTexture(mat.emissiveTexture.value());
        // }
    }
    for(auto& t : convertThread) {
        t.join();
    }
    auto last_transcode_time = std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - tStart).count();
    cout << "total converting time " <<  last_transcode_time << endl;

    std::ofstream outputFile(outputFilename, std::ios::binary);
    if (!outputFile) {
        die("Error opening file for writing!");
        return 1;
    }

    cout << "Writing output file " << outputFilename << "...\n";
    auto header = ZScene::Header {
        .version = 1,
        .headersSize = sizeof(ZScene::Header),
        .imagesCount = 2,
    };
    header.magic[0] = ZScene::Header::MAGIC[0];
    header.magic[1] = ZScene::Header::MAGIC[1];
    header.magic[2] = ZScene::Header::MAGIC[2];
    header.magic[3] = ZScene::Header::MAGIC[3];

    auto imageHeaders = vector<ZScene::ImageHeader> {2};
    auto mipHeaders = vector<vector<ZScene::MipLevelHeader>>{2};

    uint64_t dataOffset{0};
    for (auto imageIndex = 0; imageIndex < 2; imageIndex++) {
        uint64_t mipOffset{0};
        mipHeaders[imageIndex].resize(MipSetIn[imageIndex].m_nMipLevels);
        for(auto mipLevel = 0; mipLevel < MipSetCmp[imageIndex].m_nMipLevels; ++mipLevel) {
            mipHeaders[imageIndex][mipLevel].offset = mipOffset;
            mipHeaders[imageIndex][mipLevel].size = MipSetCmp[imageIndex].m_pMipLevelTable[mipLevel]->m_dwLinearSize;
            // std::printf("%d : %d (%d)\n", mipLevel, offset, mipHeader[mipLevel].size);
            mipOffset += mipHeaders[imageIndex][mipLevel].size;
        }
        // std::printf("total : %d\n", offset);

        imageHeaders[imageIndex].format = VK_FORMAT_BC1_RGB_SRGB_BLOCK;
        imageHeaders[imageIndex].width  = static_cast<uint32_t>(MipSetIn[imageIndex].m_nWidth);
        imageHeaders[imageIndex].height  = static_cast<uint32_t>(MipSetIn[imageIndex].m_nHeight);
        imageHeaders[imageIndex].mipLevels  = static_cast<uint32_t>(MipSetIn[imageIndex].m_nMipLevels);
        imageHeaders[imageIndex].dataOffset = dataOffset;
        imageHeaders[imageIndex].dataSize = mipOffset;
        dataOffset += mipOffset;

        header.headersSize += sizeof(ZScene::ImageHeader) + sizeof(ZScene::MipLevelHeader) * MipSetIn[imageIndex].m_nMipLevels;
    }

    header.headersSize = dataOffset;
    outputFile.write(reinterpret_cast<const char*>(&header), sizeof(ZScene::Header));
    for (auto imageIndex = 0; imageIndex < 2; imageIndex++) {
        outputFile.write(reinterpret_cast<const char*>(&imageHeaders[imageIndex]),sizeof(ZScene::ImageHeader));
        outputFile.write(reinterpret_cast<const ostream::char_type *>(mipHeaders[imageIndex].data()), mipHeaders[imageIndex].size() * sizeof(ZScene::MipLevelHeader));
    }

    for (auto imageIndex = 0; imageIndex < 2; imageIndex++)
    {
        for(auto mipLevel = 0; mipLevel < MipSetCmp[imageIndex].m_nMipLevels; ++mipLevel) {
            // std::printf("Writing image %d level %d %dx%d (%d)...\n", imageIndex, mipLevel,
                // MipSetCmp[imageIndex].m_pMipLevelTable[mipLevel]->m_nWidth,
                // MipSetCmp[imageIndex].m_pMipLevelTable[mipLevel]->m_nHeight,
                // MipSetCmp[imageIndex].m_pMipLevelTable[mipLevel]->m_dwLinearSize);
            outputFile.write(reinterpret_cast<const char*>(MipSetCmp[imageIndex].m_pMipLevelTable[mipLevel]->m_pbData),MipSetCmp[imageIndex].m_pMipLevelTable[mipLevel]->m_dwLinearSize);
            if (!outputFile) {
                die("Error writing to file!");
                return 1;
            }
        }

    }
    outputFile.close();

    for (auto imageIndex = 0; imageIndex < 2; imageIndex++) {
        CMP_FreeMipSet(&MipSetIn[imageIndex]);
        CMP_FreeMipSet(&MipSetCmp[imageIndex]);
    }

    CMP_ShutdownBCLibrary();

    return 0;
}
