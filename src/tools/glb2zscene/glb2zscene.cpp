#include <compressonator.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
import std;
using namespace std;

// Function to compress RGBA image to BC7 format using Compressonator SDK
CMP_ERROR CompressImageToBC7(const unsigned char* rgbaData, int width, int height, std::vector<unsigned char>& bc7Data)
{
    // Initialize the source texture with RGBA format
    CMP_Texture srcTexture;
    srcTexture.dwSize = sizeof(srcTexture);
    srcTexture.dwWidth = width;
    srcTexture.dwHeight = height;
    srcTexture.dwPitch = width * 4;
    srcTexture.format = CMP_FORMAT_RGBA_8888;
    srcTexture.dwDataSize = width * height * 4;
    srcTexture.pData = const_cast<CMP_BYTE*>(rgbaData);

    // Initialize the destination texture for BC7 format
    CMP_Texture destTexture;
    destTexture.dwSize = sizeof(destTexture);
    destTexture.dwWidth = width;
    destTexture.dwHeight = height;
    destTexture.dwPitch = 0; // Not required for compressed formats
    destTexture.format = CMP_FORMAT_BC1;
    destTexture.dwDataSize = CMP_CalculateBufferSize(&destTexture);
    destTexture.pData = new CMP_BYTE[destTexture.dwDataSize];

    // Set compression options
    CMP_CompressOptions options;
    memset(&options, 0, sizeof(options)); // Zero out options for default values
    options.nCompressionSpeed = CMP_Speed_Normal; // Adjust as needed (Fast, Normal, or Slow)

    // Compress the texture
    CMP_ERROR cmpStatus = CMP_ConvertTexture(&srcTexture, &destTexture, &options, nullptr);

    // Check compression status
    if (cmpStatus == CMP_OK)
    {
        // Copy compressed data to output vector
        bc7Data.resize(destTexture.dwDataSize);
        std::copy(destTexture.pData, destTexture.pData + destTexture.dwDataSize, bc7Data.begin());
    }
    else
    {
        std::cerr << "Compression failed with error code: " << cmpStatus << std::endl;
    }

    // Clean up allocated memory
    delete[] destTexture.pData;

    return cmpStatus;
}

int main()
{
    int texWidth, texHeight, texChannels;
    unsigned char* imageData = stbi_load("res/textures/worn_brick_floor_2k/worn_brick_floor_diff_2k.jpg",
        &texWidth, &texHeight,
        &texChannels, STBI_rgb_alpha);
    if (!imageData) throw new runtime_error(string{stbi_failure_reason()});

    std::vector<unsigned char> bc7Data;
    CMP_ERROR result = CompressImageToBC7(imageData, texWidth, texHeight, bc7Data);

    if (result == CMP_OK)
    {
        std::cout << "Compression successful! Compressed data size: " << bc7Data.size() << " bytes from " << texWidth*texHeight*4 << std::endl;
    }
    else
    {
        std::cerr << "Compression failed." << std::endl;
    }

    std::ofstream outputFile("worn_brick_floor_diff_2k.bin", std::ios::binary);
    if (!outputFile) {
        std::cerr << "Error opening file for writing!" << std::endl;
        return 1;
    }
    outputFile.write(reinterpret_cast<const char*>(bc7Data.data()), bc7Data.size());
    if (!outputFile) {
        std::cerr << "Error writing to file!" << std::endl;
        return 1;
    }
    outputFile.close();


    return 0;
}
