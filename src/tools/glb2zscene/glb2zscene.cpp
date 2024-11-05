#include <volk.h>
#include <compressonator.h>
import std;
using namespace std;

import z0;
using namespace z0;

void convert(const string& filepath, CMP_MipSet& MipSetIn, CMP_MipSet& MipSetCmp, CMP_FORMAT format)
{
     // https://compressonator.readthedocs.io/en/latest/developer_sdk/cmp_framework/index.html#using-the-pipeline-api-interfaces
    memset(&MipSetIn, 0, sizeof(CMP_MipSet));
    CMP_ERROR        cmp_status = CMP_LoadTexture(filepath.c_str(), &MipSetIn);
    if (cmp_status != CMP_OK) {
        std::printf("Error %d: Loading source file!\n",cmp_status);
    }

    //----------------------------------------------------------------------
    // generate mipmap level for the source image, if not already generated
    //----------------------------------------------------------------------
    if (MipSetIn.m_nMipLevels <= 1)
    {
        std::printf("Generating mimaps...\n");
        CMP_INT requestLevel = static_cast<int>(std::floor(std::log2(std::max(MipSetIn.m_nWidth, MipSetIn.m_nHeight))));

        //------------------------------------------------------------------------
        // Checks what the minimum image size will be for the requested mip levels
        // if the request is too large, an adjusted minimum size will be returns
        //------------------------------------------------------------------------
        CMP_INT nMinSize = CMP_CalcMinMipSize(MipSetIn.m_nHeight, MipSetIn.m_nWidth, requestLevel);

        //--------------------------------------------------------------
        // now that the minimum size is known, generate the miplevels
        // users can set any requested minumum size to use. The correct
        // miplevels will be set acordingly.
        //--------------------------------------------------------------
        CMP_GenerateMIPLevels(&MipSetIn, nMinSize);
    }

    //==========================
    // Set Compression Options
    //==========================
    KernelOptions   kernel_options;
    memset(&kernel_options, 0, sizeof(KernelOptions));

    kernel_options.format   = format;   // Set the format to process
    kernel_options.fquality = 1.0;     // Set the quality of the result
    kernel_options.threads  = 0;            // Auto setting

    //--------------------------------------------------------------
    // Setup a results buffer for the processed file,
    // the content will be set after the source texture is processed
    // in the call to CMP_ConvertMipTexture()
    //--------------------------------------------------------------
    memset(&MipSetCmp, 0, sizeof(CMP_MipSet));

    //===============================================
    // Compress the texture using Compressonator Lib
    //===============================================
    std::printf("Compressing image...\n");
    cmp_status = CMP_ProcessTexture(&MipSetIn, &MipSetCmp, kernel_options, nullptr);
    if (cmp_status != CMP_OK) {
        CMP_FreeMipSet(&MipSetIn);
        std::printf("Compression returned an error %d\n", cmp_status);
    }
    std::cout << "Compression successful!" << std::endl;
}

int main() {
    CMP_InitializeBCLibrary();

    CMP_MipSet MipSetIn[2];
    CMP_MipSet MipSetCmp[2];
    convert("res/textures/worn_brick_floor_2k/worn_brick_floor_diff_2k.jpg", MipSetIn[0], MipSetCmp[0], CMP_FORMAT_BC1);
    convert("res/textures/plaster_brick_pattern_2k/plaster_brick_pattern_diff_2k.jpg", MipSetIn[1], MipSetCmp[1], CMP_FORMAT_BC1);

    std::ofstream outputFile("testme.zscene", std::ios::binary);
    if (!outputFile) {
        std::cerr << "Error opening file for writing!" << std::endl;
        return 1;
    }

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
            std::printf("Writing image %d level %d %dx%d (%d)...\n", imageIndex, mipLevel,
                MipSetCmp[imageIndex].m_pMipLevelTable[mipLevel]->m_nWidth,
                MipSetCmp[imageIndex].m_pMipLevelTable[mipLevel]->m_nHeight,
                MipSetCmp[imageIndex].m_pMipLevelTable[mipLevel]->m_dwLinearSize);
            outputFile.write(reinterpret_cast<const char*>(MipSetCmp[imageIndex].m_pMipLevelTable[mipLevel]->m_pbData),MipSetCmp[imageIndex].m_pMipLevelTable[mipLevel]->m_dwLinearSize);
            if (!outputFile) {
                std::cerr << "Error writing to file!" << std::endl;
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
