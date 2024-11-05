#include <volk.h>
#include <compressonator.h>
import std;
using namespace std;
import z0;
using namespace z0;

int main()
{
    CMP_InitializeBCLibrary();

    CMP_MipSet MipSetIn;
    memset(&MipSetIn, 0, sizeof(CMP_MipSet));
    CMP_ERROR        cmp_status = CMP_LoadTexture("res/textures/worn_brick_floor_2k/worn_brick_floor_diff_2k.jpg", &MipSetIn);
    if (cmp_status != CMP_OK) {
        std::printf("Error %d: Loading source file!\n",cmp_status);
        return -1;
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
        // if the request is too large, a adjusted minimum size will be returns
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

    kernel_options.format   = CMP_FORMAT_BC1;   // Set the format to process
    kernel_options.fquality = 1.0;     // Set the quality of the result
    kernel_options.threads  = 0;            // Auto setting

    //--------------------------------------------------------------
    // Setup a results buffer for the processed file,
    // the content will be set after the source texture is processed
    // in the call to CMP_ConvertMipTexture()
    //--------------------------------------------------------------
    CMP_MipSet MipSetCmp;
    memset(&MipSetCmp, 0, sizeof(CMP_MipSet));

    //===============================================
    // Compress the texture using Compressonator Lib
    //===============================================
    std::printf("Compressing image...\n");
    cmp_status = CMP_ProcessTexture(&MipSetIn, &MipSetCmp, kernel_options, nullptr);
    if (cmp_status != CMP_OK) {
        CMP_FreeMipSet(&MipSetIn);
        std::printf("Compression returned an error %d\n", cmp_status);
        return cmp_status;
    }


    if (cmp_status == CMP_OK)
    {
        std::cout << "Compression successful!" << std::endl;
    }
    else
    {
        std::cerr << "Compression failed." << std::endl;
    }

    std::ofstream outputFile("worn_brick_floor_diff_2k.zscene", std::ios::binary);
    if (!outputFile) {
        std::cerr << "Error opening file for writing!" << std::endl;
        return 1;
    }
    auto header = ZScene::Header {
        .version = 1,
        .imagesCount = 1,
    };
    header.magic[0] = ZScene::Header::MAGIC[0];
    header.magic[1] = ZScene::Header::MAGIC[1];
    header.magic[2] = ZScene::Header::MAGIC[2];
    header.magic[3] = ZScene::Header::MAGIC[3];

    auto mipHeader = vector<ZScene::MipLevelHeader>{static_cast<unsigned long long>(MipSetCmp.m_nMipLevels)};
    uint64_t offset{0};
    for(auto mipLevel = 0; mipLevel < MipSetCmp.m_nMipLevels; ++mipLevel) {
        mipHeader[mipLevel].offset = offset;
        mipHeader[mipLevel].size = MipSetCmp.m_pMipLevelTable[mipLevel]->m_dwLinearSize;
        // std::printf("%d : %d (%d)\n", mipLevel, offset, mipHeader[mipLevel].size);
        offset += mipHeader[mipLevel].size;
    }
    std::printf("total : %d\n", offset);

    const auto imageHeader = ZScene::ImageHeader {
        .format = VK_FORMAT_BC1_RGB_SRGB_BLOCK,
        .width  = static_cast<uint32_t>(MipSetIn.m_nWidth),
        .height  = static_cast<uint32_t>(MipSetIn.m_nHeight),
        .mipLevels  = static_cast<uint32_t>(MipSetIn.m_nMipLevels),
        .dataSize = offset,
    };

    outputFile.write(reinterpret_cast<const char*>(&header), sizeof(ZScene::Header));
    outputFile.write(reinterpret_cast<const char*>(&imageHeader),sizeof(ZScene::ImageHeader));
    outputFile.write(reinterpret_cast<const ostream::char_type *>(mipHeader.data()), mipHeader.size() * sizeof(ZScene::MipLevelHeader));

    // std::printf("%d\n", sizeof(ZScene::Header) + sizeof(ZScene::ImageHeader) + mipHeader.size());
    for(auto mipLevel = 0; mipLevel < MipSetCmp.m_nMipLevels; ++mipLevel) {
        std::printf("Writing level %d %dx%d (%d)...\n", mipLevel,
            MipSetCmp.m_pMipLevelTable[mipLevel]->m_nWidth,
            MipSetCmp.m_pMipLevelTable[mipLevel]->m_nHeight,
            MipSetCmp.m_pMipLevelTable[mipLevel]->m_dwLinearSize);
        outputFile.write(reinterpret_cast<const char*>(MipSetCmp.m_pMipLevelTable[mipLevel]->m_pbData),MipSetCmp.m_pMipLevelTable[mipLevel]->m_dwLinearSize);
        if (!outputFile) {
            std::cerr << "Error writing to file!" << std::endl;
            return 1;
        }
    }
    outputFile.close();

    CMP_FreeMipSet(&MipSetIn);
    CMP_FreeMipSet(&MipSetCmp);
    CMP_ShutdownBCLibrary();

    return 0;
}
